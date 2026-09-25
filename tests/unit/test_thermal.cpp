#include <catch2/catch_test_macros.hpp>
#include "resilience/thermal/building_thermal.hpp"
#include "resilience/environment/environment.hpp"

using namespace resilience::thermal;
using namespace resilience::environment;
using namespace resilience::units;

TEST_CASE("Building thermal steps without NaN", "[thermal]") {
    BuildingThermalModel model;
    EnvironmentalState env;
    env.ambient_temperature = Celsius{35.0};
    env.solar_radiation_w_m2 = 700.0;
    for (int i = 0; i < 100; ++i) {
        model.step(env, Second{300.0});
    }
    REQUIRE(std::isfinite(model.state().indoor_temperature.value));
    REQUIRE(model.state().hvac_electrical_power.value >= 0.0);
}

TEST_CASE("Higher outdoor temperature increases cooling load", "[thermal]") {
    BuildingThermalModel cool;
    BuildingThermalModel hot;
    EnvironmentalState env_cool;
    env_cool.ambient_temperature = Celsius{25.0};
    EnvironmentalState env_hot;
    env_hot.ambient_temperature = Celsius{40.0};
    env_hot.solar_radiation_w_m2 = 800.0;

    for (int i = 0; i < 50; ++i) {
        cool.step(env_cool, Second{300.0});
        hot.step(env_hot, Second{300.0});
    }
    REQUIRE(hot.state().cooling_load.value >= cool.state().cooling_load.value);
}

TEST_CASE("Steady state no HVAC is above outdoor when gains present", "[thermal]") {
    BuildingThermalModel model;
    EnvironmentalState env;
    env.ambient_temperature = Celsius{20.0};
    env.solar_radiation_w_m2 = 500.0;
    auto ss = model.steady_state_no_hvac(env);
    REQUIRE(ss.value > env.ambient_temperature.value);
}
