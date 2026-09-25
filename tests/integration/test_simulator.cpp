#include <catch2/catch_test_macros.hpp>
#include "resilience/simulation/simulator.hpp"
#include "resilience/electrical/network.hpp"
#include "resilience/thermal/building_thermal.hpp"

using namespace resilience;

TEST_CASE("End-to-end short simulation runs", "[integration]") {
    simulation::SimulationConfig cfg;
    cfg.start = units::Second{0.0};
    cfg.end = units::Second{3600.0};  // 1 hour
    cfg.timestep = units::Second{300.0};
    cfg.seed = 123;
    cfg.scenario_name = "test";

    environment::SyntheticWeather::Params wp;
    wp.seed = 123;
    wp.anomaly = units::Celsius{5.0};

    simulation::Simulator sim(cfg);
    sim.set_weather(environment::SyntheticWeather(wp));

    electrical::ElectricalNetwork net;
    electrical::Bus slack;
    slack.id = "slack";
    slack.is_slack = true;
    slack.generation_kw = 10000.0;
    net.add_bus(slack);
    electrical::Bus b1;
    b1.id = "b1";
    b1.load_kw = 2000.0;
    net.add_bus(b1);
    electrical::Branch br;
    br.id = "l1";
    br.from = "slack";
    br.to = "b1";
    br.susceptance_pu = 15.0;
    br.capacity_kw = 5000.0;
    net.add_branch(br);
    electrical::Transformer t;
    t.id = "xfmr";
    t.bus_id = "b1";
    t.rated_capacity_kw = 3000.0;
    t.health = 0.8;
    t.age_years = 20.0;
    net.add_transformer(t);
    sim.set_network(std::move(net));

    thermal::BuildingThermalModel b;
    sim.add_building(std::move(b));

    auto result = sim.run();
    REQUIRE(result.performance_trace.size() > 0);
    REQUIRE(result.max_ambient_temp_c > 0.0);
    REQUIRE(result.metrics.contains("min_performance"));
}
