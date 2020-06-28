#!/bin/sh

# Exit if any command fails.
set -e

rm -rf ./build
rm -f *.gcov
rm coverage.info

mkdir -p build && cd build

cmake -DCODE_COVERAGE=ON ..
cmake --build .

./tests
