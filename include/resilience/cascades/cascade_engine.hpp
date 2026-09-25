#pragma once

#include "resilience/electrical/network.hpp"
#include "resilience/electrical/transformer.hpp"
#include "resilience/time/simulation_time.hpp"
#include "resilience/core/types.hpp"
#include <vector>
#include <string>
#include <random>
#include <algorithm>

namespace resilience::cascades {

using time::Second;
using time::SimulationEvent;
using time::EventType;
using time::EventQueue;
using electrical::ElectricalNetwork;
using electrical::Transformer;
using electrical::TransformerThermalModel;

struct CascadeRecord {
    Second timestamp{0.0};
    std::string component_id;
    std::string cause;
    std::string previous_state;
    std::string new_state;
    double capacity_change_kw{0.0};
    std::vector<std::string> affected;
};

class CascadeEngine {
public:
    explicit CascadeEngine(std::uint32_t seed = 42) : rng_(seed) {}

    void set_seed(std::uint32_t s) { rng_.seed(s); }

    // Check transformers for failure and schedule events / apply immediate failures
    void evaluate_transformers(ElectricalNetwork& net, Second t, Second dt,
                               EventQueue& events, std::vector<CascadeRecord>& log) {
        TransformerThermalModel thermal;
        const double dt_h = dt.value / 3600.0;

        for (auto& tr : net.transformers()) {
            if (tr.failed || !tr.in_service) continue;

            const double p_fail = TransformerThermalModel::failure_probability(tr, dt_h);
            std::uniform_real_distribution<double> u(0.0, 1.0);
            if (u(rng_) < p_fail) {
                // Fail it
                CascadeRecord rec;
                rec.timestamp = t;
                rec.component_id = tr.id;
                rec.cause = "thermal_overstress_or_aging";
                rec.previous_state = "in_service";
                rec.new_state = "failed";
                rec.capacity_change_kw = -tr.rated_capacity_kw * tr.health;
                log.push_back(rec);

                tr.failed = true;
                tr.in_service = false;
                tr.health = 0.0;

                // Redistribute: mark bus as reduced capacity (simplified)
                if (auto* bus = net.find_bus(tr.bus_id)) {
                    bus->generation_kw *= 0.5;  // crude reduction
                    // Could also shed load
                }

                SimulationEvent ev;
                ev.time = t;
                ev.type = EventType::ComponentFailure;
                ev.component_id = tr.id;
                ev.description = "Transformer failed due to thermal/aging stress";
                ev.magnitude = p_fail;
                events.schedule(ev);
            }
        }
    }

    // After a failure, check for secondary overloads and potential cascading
    void propagate(ElectricalNetwork& net, Second t, EventQueue& events,
                   std::vector<CascadeRecord>& log) {
        // Re-solve power flow
        net.solve_dc_power_flow();

        auto overloaded = net.overloaded_branches();
        for (const auto& br_id : overloaded) {
            CascadeRecord rec;
            rec.timestamp = t;
            rec.component_id = br_id;
            rec.cause = "load_redistribution_after_failure";
            rec.previous_state = "normal";
            rec.new_state = "overloaded";
            log.push_back(rec);

            SimulationEvent ev;
            ev.time = t;
            ev.type = EventType::PowerOverload;
            ev.component_id = br_id;
            ev.description = "Branch overloaded after cascade";
            events.schedule(ev);
        }

        // Simple secondary transformer stress: increase load on remaining
        for (auto& tr : net.transformers()) {
            if (tr.failed) continue;
            if (tr.load_kw > tr.rated_capacity_kw * 1.1) {
                CascadeRecord rec;
                rec.timestamp = t;
                rec.component_id = tr.id;
                rec.cause = "secondary_overload";
                rec.previous_state = "normal_load";
                rec.new_state = "overloaded";
                log.push_back(rec);
            }
        }
    }

    const std::vector<CascadeRecord>& last_log() const noexcept { return last_log_; }

private:
    std::mt19937 rng_;
    std::vector<CascadeRecord> last_log_;
};

}  // namespace resilience::cascades
