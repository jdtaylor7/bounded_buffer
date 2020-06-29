cmake_minimum_required(VERSION 3.10)

# Set project name.
project(bounded_buffer)

# Specify C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directories.
include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

# Optionally add coverage reporting.
add_library(coverage_config INTERFACE)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(coverage_config INTERFACE
        -O0
        -g
        --coverage
    )
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
        target_link_options(coverage_config INTERFACE --coverage)
    else()
        target_link_libraries(coverage_config INTERFACE --coverage)
    endif()
endif()

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    enable_testing()

    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_subdirectory(test)
endif()
