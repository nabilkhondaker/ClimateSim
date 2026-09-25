#include <catch2/catch_test_macros.hpp>
#include "resilience/electrical/network.hpp"

using namespace resilience::electrical;

TEST_CASE("Empty network solves", "[electrical]") {
    ElectricalNetwork net;
    REQUIRE(net.solve_dc_power_flow());
}

TEST_CASE("Simple two-bus power flow", "[electrical]") {
    ElectricalNetwork net;
    Bus slack;
    slack.id = "slack";
    slack.is_slack = true;
    slack.generation_kw = 1000.0;
    net.add_bus(slack);

    Bus load;
    load.id = "load";
    load.load_kw = 500.0;
    net.add_bus(load);

    Branch br;
    br.id = "line";
    br.from = "slack";
    br.to = "load";
    br.susceptance_pu = 10.0;
    br.capacity_kw = 2000.0;
    net.add_branch(br);

    REQUIRE(net.solve_dc_power_flow());
    REQUIRE(std::abs(net.branches()[0].flow_kw) > 0.0);
}

TEST_CASE("Transformer presence", "[electrical]") {
    ElectricalNetwork net;
    Bus b;
    b.id = "b1";
    net.add_bus(b);
    Transformer t;
    t.id = "t1";
    t.bus_id = "b1";
    t.rated_capacity_kw = 1000.0;
    net.add_transformer(t);
    REQUIRE(net.transformers().size() == 1);
    REQUIRE(net.find_transformer("t1") != nullptr);
}
