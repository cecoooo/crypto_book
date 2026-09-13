#!/usr/bin/env bash

set -e

BUILD_DIR="build"

if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

echo "cmake configuration..."
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON

echo "compile time..."
cmake --build "$BUILD_DIR"

echo -e "build successful"
echo "Application: $BUILD_DIR/bin/order_book_app"
echo "Benchmark:   $BUILD_DIR/bin/order_book_benchmark"
