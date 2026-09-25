#pragma once

#include "resilience/electrical/network.hpp"
#include "resilience/units/units.hpp"
#include "resilience/time/simulation_time.hpp"
#include <cmath>
#include <algorithm>

namespace resilience::electrical {

using units::Celsius;
using units::Second;

// Simplified IEEE-style transformer thermal model (adapted, not full IEEE C57.91).
// Hot-spot temperature dynamics drive aging acceleration.
// Documented assumptions in docs/electrical/transformer_model.md
class TransformerThermalModel {
public:
    struct Params {
        double rated_loss_kw{50.0};           // total losses at rated load
        double oil_time_constant_h{3.0};     // oil thermal time constant
        double hotspot_rise_rated_k{80.0};   // hotspot rise over ambient at rated
        double oil_rise_rated_k{55.0};
        double aging_ref_temp_c{110.0};       // reference for aging
        double life_hours_at_ref{180000.0};  // expected life at reference
    };

    explicit TransformerThermalModel(Params p = {}) : params_(p) {}

    void update(Transformer& t, Celsius ambient, Second dt) {
        if (t.failed || !t.in_service || dt.value <= 0.0) return;

        t.ambient_temp_c = ambient.value;
        const double load_ratio = t.load_kw / std::max(1.0, t.rated_capacity_kw);
        const double load_factor = std::max(0.0, load_ratio);

        // Approximate ultimate rises (proportional to load^1.6 or similar)
        const double oil_ult = params_.oil_rise_rated_k * std::pow(std::max(0.1, load_factor), 1.6);
        const double hs_ult = params_.hotspot_rise_rated_k * std::pow(std::max(0.1, load_factor), 1.6);

        // First-order lag toward ultimate
        const double tau_s = params_.oil_time_constant_h * 3600.0;
        const double alpha = 1.0 - std::exp(-dt.value / tau_s);

        const double oil_target = ambient.value + oil_ult;
        t.oil_temp_c = t.oil_temp_c + alpha * (oil_target - t.oil_temp_c);

        const double hs_target = ambient.value + hs_ult;
        t.hotspot_temp_c = t.hotspot_temp_c + alpha * (hs_target - t.hotspot_temp_c);

        // Aging acceleration (Arrhenius-like approximation)
        // F_aa = exp(15000/383 - 15000/(273+θ_hs))
        const double theta_k = 273.15 + t.hotspot_temp_c;
        if (theta_k > 200.0) {
            t.aging_acceleration = std::exp(15000.0 / 383.0 - 15000.0 / theta_k);
        } else {
            t.aging_acceleration = 0.01;
        }
        t.aging_acceleration = std::clamp(t.aging_acceleration, 0.01, 1000.0);

        // Accumulate damage / reduce health
        const double life_consumed =
            (t.aging_acceleration * dt.value / 3600.0) / params_.life_hours_at_ref;
        t.health = std::max(0.0, t.health - life_consumed * 0.5);  // scaled

        // Capacity derating with health and temperature
        if (t.hotspot_temp_c > 140.0) {
            t.health = std::max(0.0, t.health - 0.001 * (t.hotspot_temp_c - 140.0) * (dt.value / 3600.0));
        }
    }

    // Probability of failure given current health and stress (simple logistic)
    static double failure_probability(const Transformer& t, double dt_hours) {
        if (t.failed) return 1.0;
        const double stress = std::max(0.0, t.load_kw / t.rated_capacity_kw - 0.8);
        const double base_hazard = 1e-5 + (1.0 - t.health) * 1e-3 + stress * 5e-4;
        const double p = 1.0 - std::exp(-base_hazard * dt_hours);
        return std::clamp(p, 0.0, 1.0);
    }

private:
    Params params_;
};

}  // namespace resilience::electrical
