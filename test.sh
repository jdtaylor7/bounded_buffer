#!/bin/sh

mkdir build
cd build

# Exit if any command fails.
set -e

cmake ..
cmake --build .

./tests
