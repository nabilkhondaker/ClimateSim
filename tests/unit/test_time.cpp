#include <catch2/catch_test_macros.hpp>
#include "resilience/time/simulation_time.hpp"

using namespace resilience::time;

TEST_CASE("SimulationClock advances", "[time]") {
    SimulationClock clock(Second{0.0}, Second{1000.0}, Second{100.0});
    REQUIRE(clock.current().value == Approx(0.0));
    clock.advance();
    REQUIRE(clock.current().value == Approx(100.0));
    REQUIRE(clock.step_count() == 1);
    while (!clock.finished()) {
        clock.advance();
    }
    REQUIRE(clock.current().value == Approx(1000.0));
}

TEST_CASE("EventQueue ordering", "[time]") {
    EventQueue q;
    SimulationEvent e1;
    e1.time = Second{500.0};
    e1.description = "late";
    SimulationEvent e2;
    e2.time = Second{100.0};
    e2.description = "early";
    q.schedule(e1);
    q.schedule(e2);
    auto next = q.pop_next(Second{200.0});
    REQUIRE(next.has_value());
    REQUIRE(next->description == "early");
    auto none = q.pop_next(Second{50.0});
    REQUIRE_FALSE(none.has_value());
}
