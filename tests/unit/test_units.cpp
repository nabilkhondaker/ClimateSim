#include <catch2/catch_test_macros.hpp>
#include "resilience/units/units.hpp"

using namespace resilience::units;

TEST_CASE("Celsius Kelvin conversion", "[units]") {
    Celsius c{25.0};
    Kelvin k = to_kelvin(c);
    REQUIRE(k.value == Approx(298.15));
    Celsius c2 = to_celsius(k);
    REQUIRE(c2.value == Approx(25.0));
}

TEST_CASE("Power conversion", "[units]") {
    Kilowatt kw{5.0};
    Watt w = to_watts(kw);
    REQUIRE(w.value == Approx(5000.0));
    REQUIRE(to_kilowatts(w).value == Approx(5.0));
}

TEST_CASE("Energy conversion", "[units]") {
    KilowattHour kwh{1.0};
    Joule j = to_joules(kwh);
    REQUIRE(j.value == Approx(3.6e6));
}

TEST_CASE("Arithmetic", "[units]") {
    Watt a{100.0};
    Watt b{50.0};
    REQUIRE((a + b).value == Approx(150.0));
    REQUIRE((a - b).value == Approx(50.0));
    REQUIRE((a * 2.0).value == Approx(200.0));
}
