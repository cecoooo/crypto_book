#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD="$ROOT/build-benchmark"
LOG="$ROOT/benchmark_logs/performance.txt"

cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
cmake --build "$BUILD" --config Release --target order_book_benchmark

mkdir -p "$ROOT/benchmark_logs"
rm -f "$LOG"
rm -f "$ROOT/benchmark_logs/performance.csv"

for BUFFER in 131072 262144 524288
do
    "$BUILD/bin/order_book_benchmark" --mode data --scenario mixed --levels 250000 --updates 1000000 --runs 5 --buffer "$BUFFER" --log "$LOG"
done

"$BUILD/bin/order_book_benchmark" --mode data --scenario snapshot --levels 1000000 --runs 5 --buffer 262144 --log "$LOG"

printf '\nResults: %s\n' "$LOG"
