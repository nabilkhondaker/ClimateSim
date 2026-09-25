#pragma once

#include "resilience/units/units.hpp"
#include "resilience/time/simulation_time.hpp"
#include <cmath>
#include <random>
#include <string>
#include <vector>

namespace resilience::environment {

using units::Celsius;
using units::Second;
using units::PerUnit;

struct EnvironmentalState {
    Celsius ambient_temperature{25.0};
    double relative_humidity{0.5};       // [0,1]
    double solar_radiation_w_m2{0.0};    // W/m²
    double precipitation_mm_h{0.0};      // mm/h
    double wind_speed_m_s{0.0};
    PerUnit water_availability{1.0};     // relative available water
    double flood_depth_m{0.0};
};

// Synthetic weather generator.
// Explicitly synthetic: not a climate forecast model.
class SyntheticWeather {
public:
    struct Params {
        Celsius base_temperature{22.0};
        Celsius seasonal_amplitude{8.0};
        Celsius diurnal_amplitude{6.0};
        Celsius anomaly{0.0};            // heat-wave offset
        double stochastic_sigma{1.5};    // °C
        double drought_factor{1.0};      // multiplies water availability
        double flood_probability{0.0};
        std::uint32_t seed{42};
    };

    explicit SyntheticWeather(Params p = {}) : params_(p), rng_(p.seed) {}

    EnvironmentalState evaluate(Second t) const {
        EnvironmentalState s;
        const double days = t.value / 86400.0;
        const double hours = std::fmod(t.value / 3600.0, 24.0);

        // Seasonal (annual) + diurnal
        const double seasonal = params_.seasonal_amplitude.value *
                                std::sin(2.0 * 3.141592653589793 * days / 365.25);
        const double diurnal = params_.diurnal_amplitude.value *
                               std::sin(2.0 * 3.141592653589793 * (hours - 6.0) / 24.0);

        double temp = params_.base_temperature.value + seasonal + diurnal +
                      params_.anomaly.value;

        // Stochastic variation (reproducible for given seed + time bin)
        std::mt19937 local_rng(params_.seed + static_cast<std::uint32_t>(t.value / 300.0));
        std::normal_distribution<double> noise(0.0, params_.stochastic_sigma);
        temp += noise(local_rng);

        s.ambient_temperature = Celsius{temp};
        s.relative_humidity = std::clamp(0.55 - 0.01 * (temp - 25.0), 0.15, 0.95);
        s.solar_radiation_w_m2 = std::max(0.0, 800.0 * std::sin(3.141592653589793 * (hours - 6.0) / 12.0));
        s.water_availability = PerUnit{std::clamp(params_.drought_factor, 0.0, 1.0)};
        s.precipitation_mm_h = 0.0;
        s.wind_speed_m_s = 2.0 + 1.5 * std::abs(std::sin(days));
        s.flood_depth_m = 0.0;

        return s;
    }

    void set_anomaly(Celsius a) { params_.anomaly = a; }
    void set_drought_factor(double f) { params_.drought_factor = f; }
    const Params& params() const noexcept { return params_; }

private:
    Params params_;
    mutable std::mt19937 rng_;
};

}  // namespace resilience::environment
