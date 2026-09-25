#include <catch2/catch_test_macros.hpp>
#include "resilience/reliability/reliability.hpp"

using namespace resilience::reliability;

TEST_CASE("Exponential reliability at t=0 is 1", "[reliability]") {
    REQUIRE(exponential_reliability(0.01, 0.0) == Approx(1.0));
}

TEST_CASE("Exponential decreases with time", "[reliability]") {
    double r1 = exponential_reliability(0.01, 10.0);
    double r2 = exponential_reliability(0.01, 100.0);
    REQUIRE(r1 > r2);
    REQUIRE(r1 < 1.0);
}

TEST_CASE("Weibull reliability", "[reliability]") {
    REQUIRE(weibull_reliability(100.0, 2.0, 0.0) == Approx(1.0));
    double r = weibull_reliability(100.0, 2.0, 100.0);
    REQUIRE(r == Approx(std::exp(-1.0)));
}

TEST_CASE("Failure probability in interval", "[reliability]") {
    double p = failure_prob_interval(0.0, 10.0);
    REQUIRE(p == Approx(0.0));
    p = failure_prob_interval(1e-3, 1000.0);
    REQUIRE(p > 0.0);
    REQUIRE(p < 1.0);
}
