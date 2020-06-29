[![Build Status][travis-badge]][travis-url]
[![Codecov][codecov-badge]][codecov-url]
[![Version][version-badge]](version-url)
[![MIT License][license-badge]](LICENSE.md)

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

[travis-badge]: https://travis-ci.com/jdtaylor7/bounded_buffer.svg?branch=master
[travis-url]: https://travis-ci.com/jdtaylor7/bounded_buffer
[codecov-badge]: https://codecov.io/gh/jdtaylor7/bounded_buffer/coverage.svg?branch=master
[codecov-url]: https://codecov.io/gh/jdtaylor7/bounded_buffer
[version-badge]: https://img.shields.io/github/release/jdtaylor7/bounded_buffer.svg
[version-url]: https://github.com/jdtaylor7/bounded_buffer/releases
[license-badge]: https://img.shields.io/badge/license-MIT-007EC7.svg
