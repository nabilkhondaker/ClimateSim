#pragma once

#include "resilience/core/types.hpp"
#include "resilience/units/units.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cmath>
#include <stdexcept>

namespace resilience::electrical {

using core::ComponentId;
using core::NodeId;
using units::Watt;
using units::Kilowatt;

struct Bus {
    NodeId id;
    double voltage_pu{1.0};          // per-unit voltage magnitude (approx)
    double angle_rad{0.0};           // voltage angle
    Watt injection{0.0};             // net injection (gen positive, load negative)
    double load_kw{0.0};
    double generation_kw{0.0};
    bool is_slack{false};
    bool in_service{true};
};

struct Branch {
    EdgeId id;
    NodeId from;
    NodeId to;
    double susceptance_pu{10.0};     // B = 1/x
    double capacity_kw{1000.0};      // thermal/flow limit
    double flow_kw{0.0};
    bool in_service{true};
};

struct Transformer {
    ComponentId id;
    NodeId bus_id;
    double rated_capacity_kw{5000.0};
    double load_kw{0.0};
    double ambient_temp_c{25.0};
    double hotspot_temp_c{65.0};     // dynamic thermal state
    double oil_temp_c{55.0};
    double health{1.0};
    double age_years{10.0};
    double aging_acceleration{1.0};
    bool failed{false};
    bool in_service{true};
};

// Simplified DC power flow: P = B θ
// Assumptions documented in docs/electrical/electrical_model.md
class ElectricalNetwork {
public:
    void add_bus(Bus b) {
        bus_index_[b.id] = buses_.size();
        buses_.push_back(std::move(b));
    }

    void add_branch(Branch br) {
        branches_.push_back(std::move(br));
    }

    void add_transformer(Transformer t) {
        transformers_.push_back(std::move(t));
    }

    Bus* find_bus(const NodeId& id) {
        auto it = bus_index_.find(id);
        if (it == bus_index_.end()) return nullptr;
        return &buses_[it->second];
    }

    const Bus* find_bus(const NodeId& id) const {
        auto it = bus_index_.find(id);
        if (it == bus_index_.end()) return nullptr;
        return &buses_[it->second];
    }

    Transformer* find_transformer(const ComponentId& id) {
        for (auto& t : transformers_) {
            if (t.id == id) return &t;
        }
        return nullptr;
    }

    std::vector<Bus>& buses() noexcept { return buses_; }
    const std::vector<Bus>& buses() const noexcept { return buses_; }
    std::vector<Branch>& branches() noexcept { return branches_; }
    const std::vector<Branch>& branches() const noexcept { return branches_; }
    std::vector<Transformer>& transformers() noexcept { return transformers_; }
    const std::vector<Transformer>& transformers() const noexcept { return transformers_; }

    // Solve DC power flow. Returns true on success.
    bool solve_dc_power_flow() {
        const std::size_t n = buses_.size();
        if (n == 0) return true;

        // Identify slack
        int slack = -1;
        for (std::size_t i = 0; i < n; ++i) {
            if (buses_[i].is_slack && buses_[i].in_service) {
                slack = static_cast<int>(i);
                break;
            }
        }
        if (slack < 0) {
            // pick first in-service bus
            for (std::size_t i = 0; i < n; ++i) {
                if (buses_[i].in_service) {
                    slack = static_cast<int>(i);
                    buses_[i].is_slack = true;
                    break;
                }
            }
        }
        if (slack < 0) return false;

        // Build reduced B matrix and P vector (exclude slack)
        std::vector<int> map(n, -1);
        int m = 0;
        for (std::size_t i = 0; i < n; ++i) {
            if (static_cast<int>(i) != slack && buses_[i].in_service) {
                map[i] = m++;
            }
        }
        if (m == 0) {
            buses_[slack].angle_rad = 0.0;
            return true;
        }

        Eigen::MatrixXd B = Eigen::MatrixXd::Zero(m, m);
        Eigen::VectorXd P = Eigen::VectorXd::Zero(m);

        for (std::size_t i = 0; i < n; ++i) {
            if (map[i] < 0) continue;
            // net injection in pu (base 100 MVA for scaling, but we work in kW consistent units)
            // For simplicity we treat kW numbers directly with susceptance scaled.
            P(map[i]) = (buses_[i].generation_kw - buses_[i].load_kw) / 1000.0;  // rough MW scale
        }

        for (const auto& br : branches_) {
            if (!br.in_service) continue;
            auto it_f = bus_index_.find(br.from);
            auto it_t = bus_index_.find(br.to);
            if (it_f == bus_index_.end() || it_t == bus_index_.end()) continue;
            int i = static_cast<int>(it_f->second);
            int j = static_cast<int>(it_t->second);
            if (!buses_[i].in_service || !buses_[j].in_service) continue;

            const double b = br.susceptance_pu;
            if (map[i] >= 0) B(map[i], map[i]) += b;
            if (map[j] >= 0) B(map[j], map[j]) += b;
            if (map[i] >= 0 && map[j] >= 0) {
                B(map[i], map[j]) -= b;
                B(map[j], map[i]) -= b;
            }
        }

        // Solve B θ = P
        Eigen::VectorXd theta = B.colPivHouseholderQr().solve(P);

        // Assign angles
        for (std::size_t i = 0; i < n; ++i) {
            if (static_cast<int>(i) == slack) {
                buses_[i].angle_rad = 0.0;
            } else if (map[i] >= 0) {
                buses_[i].angle_rad = theta(map[i]);
            }
        }

        // Compute branch flows
        for (auto& br : branches_) {
            if (!br.in_service) {
                br.flow_kw = 0.0;
                continue;
            }
            auto it_f = bus_index_.find(br.from);
            auto it_t = bus_index_.find(br.to);
            if (it_f == bus_index_.end() || it_t == bus_index_.end()) {
                br.flow_kw = 0.0;
                continue;
            }
            const double theta_f = buses_[it_f->second].angle_rad;
            const double theta_t = buses_[it_t->second].angle_rad;
            br.flow_kw = br.susceptance_pu * (theta_f - theta_t) * 1000.0;  // back to kW scale
        }

        // Update transformer loads from bus load
        for (auto& t : transformers_) {
            if (t.failed || !t.in_service) {
                t.load_kw = 0.0;
                continue;
            }
            if (auto* bus = find_bus(t.bus_id)) {
                t.load_kw = bus->load_kw;
            }
        }

        return true;
    }

    // Total system load and generation
    double total_load_kw() const {
        double s = 0.0;
        for (const auto& b : buses_) {
            if (b.in_service) s += b.load_kw;
        }
        return s;
    }

    double total_generation_kw() const {
        double s = 0.0;
        for (const auto& b : buses_) {
            if (b.in_service) s += b.generation_kw;
        }
        return s;
    }

    // Overloaded branches
    std::vector<std::string> overloaded_branches() const {
        std::vector<std::string> out;
        for (const auto& br : branches_) {
            if (br.in_service && std::abs(br.flow_kw) > br.capacity_kw * 1.01) {
                out.push_back(br.id);
            }
        }
        return out;
    }

private:
    std::vector<Bus> buses_;
    std::vector<Branch> branches_;
    std::vector<Transformer> transformers_;
    std::unordered_map<NodeId, std::size_t> bus_index_;
};

}  // namespace resilience::electrical
