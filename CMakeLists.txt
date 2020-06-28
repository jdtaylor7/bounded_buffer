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

message("CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

# Optionally add coverage reporting.
add_library(coverage_config INTERFACE)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message("CODE_COVERAGE flags added")
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

# add_subdirectory(include)

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    message("adding the test subdirectory")
    enable_testing()

    find_package(GTest REQUIRED)
    include(GoogleTest)

    # add_subdirectory(test)

    # Add test executables.
    add_executable(tests test/bounded_buffer_test.cpp)

    # Add compiler/linker options.
    # target_compile_options(tests PRIVATE -O2)

    # Link libraries.
    target_link_libraries(tests GTest::GTest GTest::Main coverage_config)

    gtest_discover_tests(tests)
endif()

message("CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message("CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
