# Simple benchmark executable (no external Google Benchmark dependency for portability)
add_executable(resilience_bench micro/bench_thermal.cpp)
target_link_libraries(resilience_bench PRIVATE resilience::core)
target_include_directories(resilience_bench PRIVATE ${CMAKE_SOURCE_DIR}/include)
