#pragma once

#include "resilience/time/simulation_time.hpp"
#include "resilience/environment/environment.hpp"
#include "resilience/thermal/building_thermal.hpp"
#include "resilience/electrical/network.hpp"
#include "resilience/electrical/transformer.hpp"
#include "resilience/cascades/cascade_engine.hpp"
#include "resilience/core/types.hpp"
#include "resilience/metrics/resilience_metrics.hpp"
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <nlohmann/json.hpp>

namespace resilience::simulation {

using time::SimulationClock;
using time::EventQueue;
using time::Second;
using environment::SyntheticWeather;
using environment::EnvironmentalState;
using thermal::BuildingThermalModel;
using electrical::ElectricalNetwork;
using electrical::TransformerThermalModel;
using cascades::CascadeEngine;
using cascades::CascadeRecord;

struct SimulationConfig {
    Second start{0.0};
    Second end{7.0 * 86400.0};       // 7 days default
    Second timestep{300.0};          // 5 min
    std::uint32_t seed{42};
    std::string scenario_name{"baseline"};
    bool enable_cascades{true};
    bool enable_thermal{true};
    bool enable_degradation{true};
};

struct SimulationResult {
    std::string scenario_name;
    std::uint32_t seed{0};
    double total_energy_kwh{0.0};
    double max_ambient_temp_c{0.0};
    double min_performance{1.0};
    double recovery_time_h{0.0};
    std::size_t failure_count{0};
    std::vector<CascadeRecord> cascade_log;
    std::vector<double> performance_trace;
    std::vector<double> temperature_trace;
    nlohmann::json metrics;
};

class Simulator {
public:
    explicit Simulator(SimulationConfig cfg = {})
        : config_(std::move(cfg)),
          clock_(config_.start, config_.end, config_.timestep),
          weather_(),
          cascade_(config_.seed) {
        weather_.set_anomaly(units::Celsius{0.0});
    }

    void set_config(const SimulationConfig& c) {
        config_ = c;
        clock_ = SimulationClock(c.start, c.end, c.timestep);
        cascade_.set_seed(c.seed);
    }

    void set_weather(const SyntheticWeather& w) { weather_ = w; }
    void set_network(ElectricalNetwork net) { network_ = std::move(net); }
    void add_building(BuildingThermalModel b) { buildings_.push_back(std::move(b)); }

    ElectricalNetwork& network() noexcept { return network_; }
    const ElectricalNetwork& network() const noexcept { return network_; }

    SimulationResult run() {
        SimulationResult result;
        result.scenario_name = config_.scenario_name;
        result.seed = config_.seed;
        result.min_performance = 1.0;

        clock_ = SimulationClock(config_.start, config_.end, config_.timestep);
        events_.clear();
        cascade_log_.clear();

        TransformerThermalModel thermal_model;
        double performance = 1.0;
        double max_temp = -1e9;

        while (!clock_.finished()) {
            const Second t = clock_.current();
            const auto env = weather_.evaluate(t);
            max_temp = std::max(max_temp, env.ambient_temperature.value);

            // Thermal buildings -> electrical load
            double total_hvac_kw = 0.0;
            if (config_.enable_thermal) {
                for (auto& b : buildings_) {
                    b.step(env, config_.timestep);
                    total_hvac_kw += b.state().hvac_electrical_power.value / 1000.0;
                    result.total_energy_kwh += b.state().hvac_electrical_power.value *
                                               config_.timestep.value / 3.6e6;
                }
            }

            // Update bus loads (simplified: distribute HVAC to load buses)
            for (auto& bus : network_.buses()) {
                if (!bus.is_slack && bus.in_service) {
                    bus.load_kw = 200.0 + total_hvac_kw / std::max(1.0, static_cast<double>(network_.buses().size() - 1));
                }
            }

            // Power flow
            network_.solve_dc_power_flow();

            // Transformer thermal + degradation
            if (config_.enable_degradation) {
                for (auto& tr : network_.transformers()) {
                    thermal_model.update(tr, env.ambient_temperature, config_.timestep);
                }
            }

            // Cascades / failures
            if (config_.enable_cascades) {
                cascade_.evaluate_transformers(network_, t, config_.timestep, events_, cascade_log_);
                cascade_.propagate(network_, t, events_, cascade_log_);
            }

            // Process pending events at this time
            while (auto ev = events_.pop_next(t)) {
                // already applied in evaluate
            }

            // Performance metric: fraction of load served / healthy capacity
            double capacity = 0.0;
            double load = network_.total_load_kw();
            for (const auto& tr : network_.transformers()) {
                if (!tr.failed && tr.in_service) {
                    capacity += tr.rated_capacity_kw * tr.health;
                }
            }
            performance = (capacity > 0.0) ? std::min(1.0, capacity / std::max(1.0, load + capacity * 0.1)) : 0.0;
            // more simply: 1 - fraction failed
            std::size_t failed = 0;
            for (const auto& tr : network_.transformers()) {
                if (tr.failed) ++failed;
            }
            performance = 1.0 - static_cast<double>(failed) / std::max(1.0, static_cast<double>(network_.transformers().size()));
            result.min_performance = std::min(result.min_performance, performance);
            result.performance_trace.push_back(performance);
            result.temperature_trace.push_back(env.ambient_temperature.value);

            clock_.advance();
        }

        result.max_ambient_temp_c = max_temp;
        result.failure_count = cascade_log_.size();
        result.cascade_log = cascade_log_;
        result.metrics["total_energy_kwh"] = result.total_energy_kwh;
        result.metrics["max_ambient_temp_c"] = result.max_ambient_temp_c;
        result.metrics["min_performance"] = result.min_performance;
        result.metrics["failure_count"] = result.failure_count;
        result.metrics["scenario"] = result.scenario_name;
        result.metrics["seed"] = result.seed;

        return result;
    }

    void export_json(const SimulationResult& res, const std::string& path) const {
        nlohmann::json j;
        j["scenario"] = res.scenario_name;
        j["seed"] = res.seed;
        j["metrics"] = res.metrics;
        j["performance_trace"] = res.performance_trace;
        j["temperature_trace"] = res.temperature_trace;
        j["failures"] = nlohmann::json::array();
        for (const auto& c : res.cascade_log) {
            j["failures"].push_back({
                {"time_s", c.timestamp.value},
                {"component", c.component_id},
                {"cause", c.cause},
                {"new_state", c.new_state}
            });
        }
        std::ofstream ofs(path);
        ofs << j.dump(2);
    }

private:
    SimulationConfig config_;
    SimulationClock clock_;
    EventQueue events_;
    SyntheticWeather weather_;
    ElectricalNetwork network_;
    std::vector<BuildingThermalModel> buildings_;
    CascadeEngine cascade_;
    std::vector<CascadeRecord> cascade_log_;
};

}  // namespace resilience::simulation
