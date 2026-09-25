add_executable(resilience-sim ../src/cli/main.cpp)
target_link_libraries(resilience-sim
    PRIVATE
        resilience::core
        spdlog::spdlog
)
target_include_directories(resilience-sim PRIVATE ${CMAKE_SOURCE_DIR}/include)
# Eigen / json come via resilience_core
set_project_warnings(resilience-sim)
install(TARGETS resilience-sim RUNTIME DESTINATION bin)
