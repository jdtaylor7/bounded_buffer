# Add test executables.
add_executable(tests bounded_buffer_test.cpp)

# Add compiler/linker options.
if(NOT CODE_COVERAGE)
    target_compile_options(tests PRIVATE -O2)
endif()

# Link libraries.
target_link_libraries(tests GTest::GTest GTest::Main coverage_config)

gtest_discover_tests(tests)
