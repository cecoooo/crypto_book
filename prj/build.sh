#!/usr/bin/env bash

set -o pipefail

BUILD_DIR="build"
BUILD_TYPE="Debug"

mkdir -p "$BUILD_DIR"

if ! cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"; then
    echo
    echo "CMake configuration failed."
    exit 1
fi

if ! cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"; then
    echo
    echo "Build failed."
    exit 1
fi

echo
echo "Build completed successfully."
echo "Executable: $BUILD_DIR/bin/order_book_app"