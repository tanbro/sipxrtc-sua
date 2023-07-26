#!/usr/bin/env bash

set -e

mkdir -p build
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build --config Release
