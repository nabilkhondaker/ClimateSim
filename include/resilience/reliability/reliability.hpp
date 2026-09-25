#pragma once

#include <cmath>
#include <algorithm>

namespace resilience::reliability {

// Exponential reliability: R(t) = exp(-λ t)
inline double exponential_reliability(double lambda, double t) {
    if (lambda < 0.0 || t < 0.0) return 0.0;
    return std::exp(-lambda * t);
}

// Weibull reliability: R(t) = exp(-(t/η)^β)
inline double weibull_reliability(double eta, double beta, double t) {
    if (eta <= 0.0 || beta <= 0.0 || t < 0.0) return 0.0;
    return std::exp(-std::pow(t / eta, beta));
}

// Hazard rate for Weibull
inline double weibull_hazard(double eta, double beta, double t) {
    if (eta <= 0.0 || beta <= 0.0 || t < 0.0) return 0.0;
    if (t == 0.0) return (beta >= 1.0) ? 0.0 : 1e9;
    return (beta / eta) * std::pow(t / eta, beta - 1.0);
}

// Stress-adjusted hazard: λ_eff = λ0 * stress_factor
inline double stress_adjusted_hazard(double base_lambda, double stress_factor) {
    return base_lambda * std::max(0.0, stress_factor);
}

// Probability of failure in interval given constant hazard
inline double failure_prob_interval(double hazard, double dt) {
    if (hazard < 0.0 || dt < 0.0) return 0.0;
    return 1.0 - std::exp(-hazard * dt);
}

}  // namespace resilience::reliability
