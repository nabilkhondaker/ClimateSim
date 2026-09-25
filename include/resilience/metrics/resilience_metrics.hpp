#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace resilience::metrics {

// Resilience curve metrics from a performance time series (values in [0,1]).
struct ResilienceCurveMetrics {
    double initial_performance{1.0};
    double minimum_performance{1.0};
    double final_performance{1.0};
    double time_to_minimum{0.0};      // steps or hours depending on caller
    double recovery_time{0.0};
    double area_under_curve{0.0};     // integrated performance
    double loss_of_resilience{0.0};   // 1 - normalized AUC
};

inline ResilienceCurveMetrics compute_resilience_curve(const std::vector<double>& perf,
                                                       double dt_hours = 1.0) {
    ResilienceCurveMetrics m;
    if (perf.empty()) return m;

    m.initial_performance = perf.front();
    m.final_performance = perf.back();
    m.minimum_performance = *std::min_element(perf.begin(), perf.end());

    auto min_it = std::min_element(perf.begin(), perf.end());
    m.time_to_minimum = static_cast<double>(std::distance(perf.begin(), min_it)) * dt_hours;

    // Recovery: first time after minimum that performance recovers to 95% of initial
    const double target = 0.95 * m.initial_performance;
    m.recovery_time = static_cast<double>(perf.size()) * dt_hours;
    for (std::size_t i = static_cast<std::size_t>(std::distance(perf.begin(), min_it));
         i < perf.size(); ++i) {
        if (perf[i] >= target) {
            m.recovery_time = static_cast<double>(i) * dt_hours;
            break;
        }
    }

    // Trapezoidal integration
    double auc = 0.0;
    for (std::size_t i = 1; i < perf.size(); ++i) {
        auc += 0.5 * (perf[i - 1] + perf[i]) * dt_hours;
    }
    m.area_under_curve = auc;
    const double max_auc = m.initial_performance * static_cast<double>(perf.size() - 1) * dt_hours;
    m.loss_of_resilience = (max_auc > 0.0) ? 1.0 - (auc / max_auc) : 0.0;

    return m;
}

}  // namespace resilience::metrics
