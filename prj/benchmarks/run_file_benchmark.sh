#!/usr/bin/env bash

set -e

if [ -z "$2" ]; then
    echo "Usage: ./run_file_benchmark.sh SNAPSHOT_FILE UPDATES_FILE"
    exit 1
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build-benchmark"
LOG="$ROOT/benchmark_logs/file_performance.txt"

cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON

cmake --build "$BUILD" --target order_book_benchmark

if [ -f "$LOG" ]; then
    rm "$LOG"
fi

mkdir -p "$(dirname "$LOG")"
"$BUILD/bin/order_book_benchmark" --mode files --snapshot "$1" --updates-file "$2" --runs 5 --log "$LOG"

echo -e "\nResults: $LOG"