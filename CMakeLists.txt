cmake_minimum_required(VERSION 3.20)
project(ClimateInfrastructureResilienceSimulator
    VERSION 0.1.0
    DESCRIPTION "C++ simulation framework for infrastructure resilience under environmental stress"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RESILIENCE_BUILD_TESTS "Build unit and integration tests" ON)
option(RESILIENCE_BUILD_BENCHMARKS "Build benchmarks" ON)
option(RESILIENCE_BUILD_APPS "Build applications" ON)
option(RESILIENCE_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)
option(RESILIENCE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ProjectOptions)
include(CompilerWarnings)
include(Dependencies)
include(Sanitizers)

add_library(resilience_core)
add_library(resilience::core ALIAS resilience_core)

target_include_directories(resilience_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

file(GLOB_RECURSE RESILIENCE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
list(FILTER RESILIENCE_SOURCES EXCLUDE REGEX ".*/cli/.*")

target_sources(resilience_core PRIVATE ${RESILIENCE_SOURCES})

# Link available deps
set(RESILIENCE_LIBS)
if(TARGET Eigen3::Eigen)
    list(APPEND RESILIENCE_LIBS Eigen3::Eigen)
elseif(EIGEN3_INCLUDE_DIR)
    target_include_directories(resilience_core PUBLIC ${EIGEN3_INCLUDE_DIR})
endif()
if(TARGET nlohmann_json::nlohmann_json)
    list(APPEND RESILIENCE_LIBS nlohmann_json::nlohmann_json)
endif()
if(TARGET yaml-cpp)
    list(APPEND RESILIENCE_LIBS yaml-cpp)
elseif(TARGET yaml-cpp::yaml-cpp)
    list(APPEND RESILIENCE_LIBS yaml-cpp::yaml-cpp)
endif()
if(TARGET spdlog::spdlog)
    list(APPEND RESILIENCE_LIBS spdlog::spdlog)
endif()
if(TARGET fmt::fmt)
    list(APPEND RESILIENCE_LIBS fmt::fmt)
endif()

target_link_libraries(resilience_core PUBLIC ${RESILIENCE_LIBS})

if(RESILIENCE_ENABLE_SANITIZERS)
    enable_sanitizers(resilience_core)
endif()
set_project_warnings(resilience_core)

if(RESILIENCE_BUILD_APPS)
    add_subdirectory(apps)
endif()

if(RESILIENCE_BUILD_TESTS AND Catch2_FOUND)
    enable_testing()
    add_subdirectory(tests)
endif()

if(RESILIENCE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

message(STATUS "Climate Infrastructure Resilience Simulator ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build tests: ${RESILIENCE_BUILD_TESTS}")
message(STATUS "  Build apps: ${RESILIENCE_BUILD_APPS}")
