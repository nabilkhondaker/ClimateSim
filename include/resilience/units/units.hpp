#pragma once

#include <cmath>
#include <ostream>
#include <string_view>

namespace resilience::units {

// Lightweight strong-type wrappers for common engineering quantities.
// Conversions are explicit to avoid silent unit mixing.

struct Celsius {
    double value{0.0};
    constexpr explicit Celsius(double v = 0.0) noexcept : value(v) {}
    constexpr Celsius operator+(Celsius o) const noexcept { return Celsius{value + o.value}; }
    constexpr Celsius operator-(Celsius o) const noexcept { return Celsius{value - o.value}; }
    constexpr Celsius operator*(double s) const noexcept { return Celsius{value * s}; }
    constexpr Celsius operator/(double s) const noexcept { return Celsius{value / s}; }
    constexpr bool operator<(Celsius o) const noexcept { return value < o.value; }
    constexpr bool operator>(Celsius o) const noexcept { return value > o.value; }
};

struct Kelvin {
    double value{0.0};
    constexpr explicit Kelvin(double v = 0.0) noexcept : value(v) {}
    constexpr Kelvin operator+(Kelvin o) const noexcept { return Kelvin{value + o.value}; }
    constexpr Kelvin operator-(Kelvin o) const noexcept { return Kelvin{value - o.value}; }
};

constexpr Kelvin to_kelvin(Celsius c) noexcept { return Kelvin{c.value + 273.15}; }
constexpr Celsius to_celsius(Kelvin k) noexcept { return Celsius{k.value - 273.15}; }

struct Watt {
    double value{0.0};
    constexpr explicit Watt(double v = 0.0) noexcept : value(v) {}
    constexpr Watt operator+(Watt o) const noexcept { return Watt{value + o.value}; }
    constexpr Watt operator-(Watt o) const noexcept { return Watt{value - o.value}; }
    constexpr Watt operator*(double s) const noexcept { return Watt{value * s}; }
    constexpr Watt operator/(double s) const noexcept { return Watt{value / s}; }
    constexpr bool operator<(Watt o) const noexcept { return value < o.value; }
    constexpr bool operator>(Watt o) const noexcept { return value > o.value; }
};

struct Kilowatt {
    double value{0.0};
    constexpr explicit Kilowatt(double v = 0.0) noexcept : value(v) {}
    constexpr Kilowatt operator+(Kilowatt o) const noexcept { return Kilowatt{value + o.value}; }
    constexpr Kilowatt operator-(Kilowatt o) const noexcept { return Kilowatt{value - o.value}; }
    constexpr Kilowatt operator*(double s) const noexcept { return Kilowatt{value * s}; }
};

constexpr Watt to_watts(Kilowatt kw) noexcept { return Watt{kw.value * 1000.0}; }
constexpr Kilowatt to_kilowatts(Watt w) noexcept { return Kilowatt{w.value / 1000.0}; }

struct Joule {
    double value{0.0};
    constexpr explicit Joule(double v = 0.0) noexcept : value(v) {}
    constexpr Joule operator+(Joule o) const noexcept { return Joule{value + o.value}; }
    constexpr Joule operator*(double s) const noexcept { return Joule{value * s}; }
};

struct KilowattHour {
    double value{0.0};
    constexpr explicit KilowattHour(double v = 0.0) noexcept : value(v) {}
};

constexpr Joule to_joules(KilowattHour kwh) noexcept { return Joule{kwh.value * 3.6e6}; }
constexpr KilowattHour to_kwh(Joule j) noexcept { return KilowattHour{j.value / 3.6e6}; }

struct Volt {
    double value{0.0};
    constexpr explicit Volt(double v = 0.0) noexcept : value(v) {}
};

struct Ampere {
    double value{0.0};
    constexpr explicit Ampere(double v = 0.0) noexcept : value(v) {}
};

struct Ohm {
    double value{0.0};
    constexpr explicit Ohm(double v = 0.0) noexcept : value(v) {}
};

struct Second {
    double value{0.0};
    constexpr explicit Second(double v = 0.0) noexcept : value(v) {}
    constexpr Second operator+(Second o) const noexcept { return Second{value + o.value}; }
    constexpr Second operator-(Second o) const noexcept { return Second{value - o.value}; }
    constexpr Second operator*(double s) const noexcept { return Second{value * s}; }
    constexpr bool operator<(Second o) const noexcept { return value < o.value; }
    constexpr bool operator<=(Second o) const noexcept { return value <= o.value; }
    constexpr bool operator>(Second o) const noexcept { return value > o.value; }
};

struct Meter {
    double value{0.0};
    constexpr explicit Meter(double v = 0.0) noexcept : value(v) {}
};

struct CubicMeter {
    double value{0.0};
    constexpr explicit CubicMeter(double v = 0.0) noexcept : value(v) {}
    constexpr CubicMeter operator+(CubicMeter o) const noexcept { return CubicMeter{value + o.value}; }
    constexpr CubicMeter operator-(CubicMeter o) const noexcept { return CubicMeter{value - o.value}; }
    constexpr CubicMeter operator*(double s) const noexcept { return CubicMeter{value * s}; }
    constexpr bool operator<(CubicMeter o) const noexcept { return value < o.value; }
};

struct CubicMeterPerSecond {
    double value{0.0};
    constexpr explicit CubicMeterPerSecond(double v = 0.0) noexcept : value(v) {}
};

struct Pascal {
    double value{0.0};
    constexpr explicit Pascal(double v = 0.0) noexcept : value(v) {}
};

struct PerUnit {
    double value{0.0};  // [0,1] typically
    constexpr explicit PerUnit(double v = 0.0) noexcept : value(v) {}
    constexpr PerUnit operator*(double s) const noexcept { return PerUnit{value * s}; }
};

inline std::ostream& operator<<(std::ostream& os, Celsius c) {
    return os << c.value << " °C";
}
inline std::ostream& operator<<(std::ostream& os, Watt w) {
    return os << w.value << " W";
}
inline std::ostream& operator<<(std::ostream& os, Second s) {
    return os << s.value << " s";
}

}  // namespace resilience::units
