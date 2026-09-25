#pragma once

#include "resilience/units/units.hpp"
#include "resilience/environment/environment.hpp"
#include "resilience/time/simulation_time.hpp"
#include <cmath>
#include <algorithm>

namespace resilience::thermal {

using units::Celsius;
using units::Watt;
using units::Second;
using units::PerUnit;

// Simple single-zone RC thermal model:
// C * dT/dt = Q_internal + Q_solar + UA*(T_out - T_in) - Q_hvac
//
// Units: C in J/K, UA in W/K, powers in W, temperatures in °C.
struct BuildingThermalParams {
    double thermal_capacitance_j_per_k{5.0e7};  // ~ typical residential zone
    double envelope_ua_w_per_k{150.0};          // overall heat transfer coeff
    double solar_aperture_m2{8.0};
    double internal_gain_w{500.0};
    Celsius setpoint_cooling{24.0};
    Celsius setpoint_heating{20.0};
    Watt hvac_max_cooling{12000.0};
    Watt hvac_max_heating{8000.0};
    double hvac_cop_cooling{3.0};               // coefficient of performance
    double hvac_efficiency_heating{0.95};
    double floor_area_m2{150.0};
};

struct BuildingThermalState {
    Celsius indoor_temperature{22.0};
    Watt cooling_load{0.0};
    Watt heating_load{0.0};
    Watt hvac_electrical_power{0.0};
    double overheating_hours{0.0};
    double cumulative_energy_kwh{0.0};
};

class BuildingThermalModel {
public:
    explicit BuildingThermalModel(BuildingThermalParams p = {}) : params_(p) {
        state_.indoor_temperature = Celsius{22.0};
    }

    void set_params(const BuildingThermalParams& p) { params_ = p; }
    const BuildingThermalParams& params() const noexcept { return params_; }
    BuildingThermalState& state() noexcept { return state_; }
    const BuildingThermalState& state() const noexcept { return state_; }

    // Advance one timestep using forward Euler (sufficient for this simplified model).
    // dt in seconds.
    void step(const environment::EnvironmentalState& env, Second dt) {
        if (dt.value <= 0.0) return;

        const double T_in = state_.indoor_temperature.value;
        const double T_out = env.ambient_temperature.value;
        const double UA = params_.envelope_ua_w_per_k;
        const double C = params_.thermal_capacitance_j_per_k;

        // Gains
        const double Q_solar = params_.solar_aperture_m2 * env.solar_radiation_w_m2 * 0.7;
        const double Q_internal = params_.internal_gain_w;
        const double Q_envelope = UA * (T_out - T_in);

        // Ideal HVAC (proportional with saturation)
        double Q_hvac = 0.0;
        state_.cooling_load = Watt{0.0};
        state_.heating_load = Watt{0.0};
        state_.hvac_electrical_power = Watt{0.0};

        const double error_cool = T_in - params_.setpoint_cooling.value;
        const double error_heat = params_.setpoint_heating.value - T_in;

        if (error_cool > 0.0) {
            // Need cooling
            Q_hvac = -std::min(params_.hvac_max_cooling.value,
                               error_cool * UA * 3.0 + 2000.0);  // simple proportional
            state_.cooling_load = Watt{-Q_hvac};
            state_.hvac_electrical_power =
                Watt{state_.cooling_load.value / std::max(0.5, params_.hvac_cop_cooling)};
        } else if (error_heat > 0.0) {
            Q_hvac = std::min(params_.hvac_max_heating.value,
                              error_heat * UA * 2.0 + 1000.0);
            state_.heating_load = Watt{Q_hvac};
            state_.hvac_electrical_power =
                Watt{state_.heating_load.value / std::max(0.5, params_.hvac_efficiency_heating)};
        }

        // Net heat into zone
        const double Q_net = Q_internal + Q_solar + Q_envelope + Q_hvac;
        const double dT = (Q_net / C) * dt.value;
        state_.indoor_temperature = Celsius{T_in + dT};

        // Metrics
        if (state_.indoor_temperature.value > params_.setpoint_cooling.value + 1.0) {
            state_.overheating_hours += dt.value / 3600.0;
        }
        state_.cumulative_energy_kwh +=
            (state_.hvac_electrical_power.value * dt.value) / 3.6e6;
    }

    // Steady-state indoor temperature for validation (no HVAC).
    Celsius steady_state_no_hvac(const environment::EnvironmentalState& env) const {
        // 0 = Q_int + Q_sol + UA*(Tout - Tin)  => Tin = Tout + (Qint+Qsol)/UA
        const double Q_sol = params_.solar_aperture_m2 * env.solar_radiation_w_m2 * 0.7;
        const double Tin = env.ambient_temperature.value +
                           (params_.internal_gain_w + Q_sol) / params_.envelope_ua_w_per_k;
        return Celsius{Tin};
    }

private:
    BuildingThermalParams params_;
    BuildingThermalState state_;
};

}  // namespace resilience::thermal
