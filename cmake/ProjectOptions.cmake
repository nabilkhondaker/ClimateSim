# Project-wide compile options and definitions

add_library(resilience_project_options INTERFACE)
add_library(resilience::project_options ALIAS resilience_project_options)

target_compile_features(resilience_project_options INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(resilience_project_options INTERFACE
        -fno-omit-frame-pointer
    )
endif()

# Position independent code for shared library use
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
