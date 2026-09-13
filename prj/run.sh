#!/usr/bin/env bash

EXECUTABLE="build/bin/order_book_app"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Executable not found: $EXECUTABLE"
    echo "Run ./build.sh first."
    exit 1
fi

"$EXECUTABLE"