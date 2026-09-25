#pragma once

#include "resilience/units/units.hpp"
#include <cstdint>
#include <functional>
#include <queue>
#include <vector>
#include <string>
#include <optional>

namespace resilience::time {

using units::Second;

class SimulationClock {
public:
    explicit SimulationClock(Second start = Second{0.0}, Second end = Second{86400.0},
                             Second timestep = Second{300.0})
        : start_(start), end_(end), timestep_(timestep), current_(start) {}

    Second current() const noexcept { return current_; }
    Second start() const noexcept { return start_; }
    Second end() const noexcept { return end_; }
    Second timestep() const noexcept { return timestep_; }
    Second elapsed() const noexcept { return Second{current_.value - start_.value}; }

    bool finished() const noexcept { return current_.value >= end_.value; }

    void advance() {
        current_ = Second{current_.value + timestep_.value};
        if (current_.value > end_.value) {
            current_ = end_;
        }
    }

    void set_current(Second t) noexcept { current_ = t; }

    std::uint64_t step_count() const noexcept {
        if (timestep_.value <= 0.0) return 0;
        return static_cast<std::uint64_t>((current_.value - start_.value) / timestep_.value);
    }

private:
    Second start_;
    Second end_;
    Second timestep_;
    Second current_;
};

enum class EventType {
    ComponentFailure,
    ComponentRepair,
    Maintenance,
    TemperatureSpike,
    PowerOverload,
    WaterShortage,
    BatteryDepletion,
    LoadChange,
    Intervention,
    Custom
};

struct SimulationEvent {
    Second time{0.0};
    EventType type{EventType::Custom};
    std::string component_id;
    std::string description;
    double magnitude{0.0};
    std::uint64_t sequence{0};

    bool operator>(const SimulationEvent& other) const noexcept {
        if (time.value != other.time.value) {
            return time.value > other.time.value;  // min-heap via greater
        }
        return sequence > other.sequence;
    }
};

class EventQueue {
public:
    void schedule(SimulationEvent event) {
        event.sequence = next_seq_++;
        queue_.push(std::move(event));
    }

    bool empty() const noexcept { return queue_.empty(); }

    std::optional<SimulationEvent> pop_next(Second current_time) {
        if (queue_.empty()) return std::nullopt;
        if (queue_.top().time.value > current_time.value) return std::nullopt;
        auto ev = queue_.top();
        queue_.pop();
        return ev;
    }

    std::optional<SimulationEvent> peek() const {
        if (queue_.empty()) return std::nullopt;
        return queue_.top();
    }

    void clear() {
        while (!queue_.empty()) queue_.pop();
        next_seq_ = 0;
    }

    std::size_t size() const noexcept { return queue_.size(); }

private:
    std::priority_queue<SimulationEvent, std::vector<SimulationEvent>,
                        std::greater<SimulationEvent>>
        queue_;
    std::uint64_t next_seq_{0};
};

}  // namespace resilience::time
