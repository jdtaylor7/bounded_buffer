[![Build Status][travis]][travis-url]

# Bounded Buffer

Simple templated bounded buffer for use in producer/consumer applications.
Contains interfaces for pushing/popping immediately or after waiting (either
for a specified timeout period or indefinitely). Tracks number of failed push
operations as well.

### Running Tests

Tests are written with [GoogleTest](https://github.com/google/googletest) and
can be run with [Bazel](https://bazel.build/):

`bazel test --config=linux //test:bounded_buffer`

or CMake, from within the root directory:

`./test.sh`

### License

MIT License

[travis]: https://travis-ci.com/jdtaylor7/bounded_buffer.svg?branch=master
[travis-url]: https://travis-ci.com/jdtaylor7/bounded_buffer
