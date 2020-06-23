cmake_minimum_required(VERSION 3.10)

# Set project name.
project(bounded_buffer)

# Specify C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add GoogleTest.
enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

# Add include directories.
include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${GTEST_INCLUDE_DIR}"
)

# Add compiler/linker options.
set(CXXOPTS "-O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXOPTS}")

# Add test executables.
add_executable(tests test/bounded_buffer_test.cpp)

# Link libraries.
target_link_libraries(tests GTest::GTest GTest::Main)

# Discover tests.
gtest_discover_tests(tests)
