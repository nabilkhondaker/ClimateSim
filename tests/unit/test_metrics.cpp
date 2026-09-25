#include <catch2/catch_test_macros.hpp>
#include "resilience/metrics/resilience_metrics.hpp"

using namespace resilience::metrics;

TEST_CASE("Perfect performance has zero loss", "[metrics]") {
    std::vector<double> perf(100, 1.0);
    auto m = compute_resilience_curve(perf, 1.0);
    REQUIRE(m.minimum_performance == Approx(1.0));
    REQUIRE(m.loss_of_resilience == Approx(0.0).margin(1e-6));
}

TEST_CASE("Dip reduces AUC", "[metrics]") {
    std::vector<double> perf(50, 1.0);
    for (int i = 10; i < 30; ++i) perf[i] = 0.5;
    auto m = compute_resilience_curve(perf, 1.0);
    REQUIRE(m.minimum_performance == Approx(0.5));
    REQUIRE(m.loss_of_resilience > 0.0);
}
