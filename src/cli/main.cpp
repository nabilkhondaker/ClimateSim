#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <nlohmann/json.hpp>

#include "resilience/simulation/simulator.hpp"
#include "resilience/electrical/network.hpp"
#include "resilience/thermal/building_thermal.hpp"
#include "resilience/environment/environment.hpp"
#include "resilience/metrics/resilience_metrics.hpp"
#include "resilience/core/version.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>

namespace fs = std::filesystem;
using namespace resilience;

static electrical::ElectricalNetwork make_demo_network() {
    electrical::ElectricalNetwork net;
    electrical::Bus slack;
    slack.id = "bus_slack";
    slack.is_slack = true;
    slack.generation_kw = 20000.0;
    net.add_bus(slack);

    electrical::Bus b1; b1.id = "bus_1"; b1.load_kw = 3000.0; net.add_bus(b1);
    electrical::Bus b2; b2.id = "bus_2"; b2.load_kw = 2500.0; net.add_bus(b2);
    electrical::Bus b3; b3.id = "bus_hospital"; b3.load_kw = 1500.0; net.add_bus(b3);

    electrical::Branch br1; br1.id = "line_1"; br1.from = "bus_slack"; br1.to = "bus_1";
    br1.susceptance_pu = 20.0; br1.capacity_kw = 8000.0; net.add_branch(br1);
    electrical::Branch br2; br2.id = "line_2"; br2.from = "bus_1"; br2.to = "bus_2";
    br2.susceptance_pu = 15.0; br2.capacity_kw = 5000.0; net.add_branch(br2);
    electrical::Branch br3; br3.id = "line_3"; br3.from = "bus_2"; br3.to = "bus_hospital";
    br3.susceptance_pu = 12.0; br3.capacity_kw = 3000.0; net.add_branch(br3);

    electrical::Transformer t1; t1.id = "xfmr_1"; t1.bus_id = "bus_1";
    t1.rated_capacity_kw = 6000.0; t1.age_years = 25.0; t1.health = 0.85; net.add_transformer(t1);
    electrical::Transformer t2; t2.id = "xfmr_2"; t2.bus_id = "bus_2";
    t2.rated_capacity_kw = 4000.0; t2.age_years = 18.0; t2.health = 0.90; net.add_transformer(t2);
    electrical::Transformer t3; t3.id = "xfmr_hospital"; t3.bus_id = "bus_hospital";
    t3.rated_capacity_kw = 2500.0; t3.age_years = 12.0; t3.health = 0.95; net.add_transformer(t3);
    return net;
}

static void add_demo_buildings(simulation::Simulator& sim, int n = 5) {
    for (int i = 0; i < n; ++i) {
        thermal::BuildingThermalParams p;
        p.internal_gain_w = 400.0 + 50.0 * i;
        p.hvac_max_cooling = units::Watt{10000.0 + 1000.0 * i};
        p.envelope_ua_w_per_k = 120.0 + 10.0 * i;
        sim.add_building(thermal::BuildingThermalModel(p));
    }
}

static void print_usage() {
    std::cout << "Climate Infrastructure Resilience Simulator " << resilience::version() << "\n"
              << "Author: " << resilience::author() << "\n\n"
              << "Usage:\n"
              << "  resilience-sim run [--scenario NAME] [--days N] [--seed S] [--output PATH]\n"
              << "  resilience-sim compare [--output PATH]\n"
              << "  resilience-sim version\n"
              << "  resilience-sim help\n\n"
              << "Scenarios: baseline, heat_wave, extreme_heat\n";
}

int main(int argc, char** argv) {
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::info);

    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string cmd = argv[1];
    if (cmd == "version" || cmd == "--version" || cmd == "-v") {
        std::cout << "Climate Infrastructure Resilience Simulator " << resilience::version() << "\n";
        std::cout << "Author: " << resilience::author() << "\n";
        return 0;
    }
    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        print_usage();
        return 0;
    }

    if (cmd == "run") {
        std::string scenario = "baseline";
        std::string output = "results/simulation_result.json";
        int days = 3;
        std::uint32_t seed = 42;
        for (int i = 2; i < argc; ++i) {
            std::string a = argv[i];
            if ((a == "--scenario" || a == "-s") && i + 1 < argc) scenario = argv[++i];
            else if ((a == "--output" || a == "-o") && i + 1 < argc) output = argv[++i];
            else if (a == "--days" && i + 1 < argc) days = std::atoi(argv[++i]);
            else if (a == "--seed" && i + 1 < argc) seed = static_cast<std::uint32_t>(std::atoi(argv[++i]));
        }

        simulation::SimulationConfig cfg;
        cfg.start = units::Second{0.0};
        cfg.end = units::Second{static_cast<double>(days) * 86400.0};
        cfg.timestep = units::Second{300.0};
        cfg.seed = seed;
        cfg.scenario_name = scenario;

        environment::SyntheticWeather::Params wp;
        wp.seed = seed;
        if (scenario == "heat_wave") {
            wp.anomaly = units::Celsius{8.0};
            wp.base_temperature = units::Celsius{28.0};
        } else if (scenario == "extreme_heat") {
            wp.anomaly = units::Celsius{12.0};
            wp.base_temperature = units::Celsius{30.0};
        }

        simulation::Simulator sim(cfg);
        sim.set_weather(environment::SyntheticWeather(wp));
        sim.set_network(make_demo_network());
        add_demo_buildings(sim, 8);

        spdlog::info("Running scenario '{}' for {} days (seed={})", scenario, days, seed);
        auto result = sim.run();

        fs::create_directories(fs::path(output).parent_path());
        sim.export_json(result, output);

        auto curve = metrics::compute_resilience_curve(result.performance_trace, 300.0 / 3600.0);
        spdlog::info("Finished. Failures: {}, min performance: {:.3f}, max temp: {:.1f} C",
                     result.failure_count, result.min_performance, result.max_ambient_temp_c);
        spdlog::info("Resilience AUC loss: {:.3f}", curve.loss_of_resilience);
        spdlog::info("Results written to {}", output);
        return 0;
    }

    if (cmd == "compare") {
        std::string compare_out = "results/comparison.json";
        for (int i = 2; i < argc; ++i) {
            if ((std::string(argv[i]) == "--output" || std::string(argv[i]) == "-o") && i + 1 < argc)
                compare_out = argv[++i];
        }
        nlohmann::json comparison;
        for (const char* scen : {"baseline", "heat_wave", "extreme_heat"}) {
            simulation::SimulationConfig cfg;
            cfg.end = units::Second{2.0 * 86400.0};
            cfg.seed = 42;
            cfg.scenario_name = scen;
            environment::SyntheticWeather::Params wp;
            wp.seed = 42;
            if (std::string(scen) == "heat_wave") wp.anomaly = units::Celsius{8.0};
            if (std::string(scen) == "extreme_heat") wp.anomaly = units::Celsius{12.0};
            simulation::Simulator sim(cfg);
            sim.set_weather(environment::SyntheticWeather(wp));
            sim.set_network(make_demo_network());
            add_demo_buildings(sim, 6);
            auto res = sim.run();
            comparison[scen] = res.metrics;
        }
        fs::create_directories(fs::path(compare_out).parent_path());
        std::ofstream ofs(compare_out);
        ofs << comparison.dump(2);
        spdlog::info("Comparison written to {}", compare_out);
        return 0;
    }

    std::cerr << "Unknown command: " << cmd << "\n";
    print_usage();
    return 1;
}
