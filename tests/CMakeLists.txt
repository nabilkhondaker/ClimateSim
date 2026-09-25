add_executable(resilience_tests
    unit/test_units.cpp
    unit/test_time.cpp
    unit/test_thermal.cpp
    unit/test_reliability.cpp
    unit/test_electrical.cpp
    unit/test_metrics.cpp
    integration/test_simulator.cpp
)

target_link_libraries(resilience_tests
    PRIVATE
        resilience::core
        Catch2::Catch2WithMain
)

target_include_directories(resilience_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_project_warnings(resilience_tests)

include(CTest)
include(Catch)
catch_discover_tests(resilience_tests)
