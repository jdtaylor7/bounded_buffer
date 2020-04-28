# Bounded Buffer

Simple templated bounded buffer for use in producer/consumer applications. Contains interfaces for pushing/popping immediately or after waiting (either for a specified timeout period or indefinitely). Tracks number of failed push operations as well.

Tests are written with [GoogleTest](https://github.com/google/googletest) and run with [Bazel](https://bazel.build/):

`bazel test //test:bounded_buffer`
