#include "resilience/thermal/building_thermal.hpp"
#include "resilience/environment/environment.hpp"
#include <chrono>
#include <iostream>

int main() {
    using namespace resilience;
    thermal::BuildingThermalModel model;
    environment::EnvironmentalState env;
    env.ambient_temperature = units::Celsius{35.0};
    env.solar_radiation_w_m2 = 700.0;

    const int N = 100000;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
        model.step(env, units::Second{300.0});
    }
    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "Thermal steps: " << N << " in " << ms << " ms ("
              << (N / (ms / 1000.0)) << " steps/s)\n";
    return 0;
}
