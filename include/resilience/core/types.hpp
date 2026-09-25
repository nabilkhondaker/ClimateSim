#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <optional>
#include <stdexcept>

namespace resilience::core {

using ComponentId = std::string;
using NodeId = std::string;
using EdgeId = std::string;

enum class ComponentType {
    Transformer,
    Generator,
    Load,
    Bus,
    Battery,
    Pump,
    Reservoir,
    Pipe,
    Building,
    Road,
    Substation,
    Unknown
};

enum class HealthState {
    Healthy,
    Degraded,
    Failed,
    UnderRepair,
    Offline
};

enum class Criticality {
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4,
    Essential = 5
};

struct ComponentState {
    ComponentId id;
    ComponentType type{ComponentType::Unknown};
    HealthState health{HealthState::Healthy};
    Criticality criticality{Criticality::Medium};
    double health_index{1.0};       // [0,1]
    double capacity_fraction{1.0};  // effective capacity multiplier
    double age_years{0.0};
    double accumulated_damage{0.0};
    bool in_service{true};
};

class ConfigurationError : public std::runtime_error {
public:
    explicit ConfigurationError(const std::string& msg) : std::runtime_error(msg) {}
};

class SimulationError : public std::runtime_error {
public:
    explicit SimulationError(const std::string& msg) : std::runtime_error(msg) {}
};

class NumericalError : public std::runtime_error {
public:
    explicit NumericalError(const std::string& msg) : std::runtime_error(msg) {}
};

class InvalidNetworkError : public std::runtime_error {
public:
    explicit InvalidNetworkError(const std::string& msg) : std::runtime_error(msg) {}
};

}  // namespace resilience::core
