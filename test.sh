#!/bin/sh

# Exit if any command fails.
set -e

rm -rf ./build

mkdir -p build && cd build

cmake -DCODE_COVERAGE=ON ..
cmake --build .

./test/tests
