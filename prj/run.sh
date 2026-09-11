#!/usr/bin/env bash

set -o pipefail

EXECUTABLE="build/bin/order_book_app"

if [[ ! -f "$EXECUTABLE" ]]; then
    echo "Executable not found: $EXECUTABLE"
    echo "Run ./build.sh first."
    exit 1
fi

if [[ ! -x "$EXECUTABLE" ]]; then
    echo "Executable is not executable: $EXECUTABLE"
    echo "Run: chmod +x $EXECUTABLE"
    exit 1
fi

"$EXECUTABLE"