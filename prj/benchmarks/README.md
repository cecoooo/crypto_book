# Benchmark suite

The benchmark target is separate from the application and uses the same `Book`,
`PriceLevelContainer`, `BookContainer`, and `Engine` implementations.

## Build

Use a Release build. Debug builds do not represent the performance of the
application.

Windows:

```text
cmake -S . -B build-benchmark -DBUILD_BENCHMARKS=ON
cmake --build build-benchmark --config Release --target order_book_benchmark
```

Linux:

```text
cmake -S . -B build-benchmark -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
cmake --build build-benchmark --target order_book_benchmark
```

## Standard benchmark

On Windows run:

```text
benchmarks\run_benchmarks.bat
```

On Linux run:

```text
chmod +x benchmarks/run_benchmarks.sh
./benchmarks/run_benchmarks.sh
```

The standard suite compares buffer sizes 131072, 262144, and 524288 with the
same deterministic mixed update stream. It then measures snapshot construction
with one million levels per side. Results are written to
`benchmark_logs/performance.txt`.

## End-to-end file benchmark

On Windows run:

```text
benchmarks\run_file_benchmark.bat input_data\snapshot.txt input_data\updates.txt
```

For a larger run, replace the paths with `lite_snapshot.txt` and
`lite_updates.txt` or with the heavy files.

The equivalent direct command on Linux is:

```text
./build-benchmark/bin/order_book_benchmark \
  --mode files \
  --snapshot input_data/snapshot.txt \
  --updates-file input_data/updates.txt \
  --runs 5 \
  --log benchmark_logs/file_performance.txt
```

## Available data scenarios

- `snapshot` measures sorting, duplicate removal, and initial book creation.
- `replace` updates prices already present in the book.
- `delete` sends zero-quantity updates for existing prices.
- `insert` adds new prices.
- `mixed` uses 50 percent replacements, 20 percent deletions, 20 percent
  insertions, and 10 percent deletions of missing prices.
- `all` runs every scenario.

Example:

```text
order_book_benchmark --mode data --scenario all --levels 250000 \
  --updates 1000000 --runs 5 --buffer 262144 \
  --log benchmark_logs/all_scenarios.txt
```

## Text report fields

- `Elapsed time` is the total measured duration in milliseconds.
- `Throughput` is levels per second, updates per second, or bytes per second.
- `Average time` is the average nanoseconds per level, update, or byte.
- `RSS before` and `RSS after` are process resident memory before and after the
  measured operation.
- `Peak RSS` is the operating system process peak. The peak is not reset
  between runs, so use a separate process when comparing memory peaks.
- `Private memory` is available on Windows and is zero on unsupported systems.
- `Container allocation` is memory reserved by the price-level vectors. It excludes
  allocator metadata, strings, maps, reader buffers, and the process runtime.
- `Active levels` and `Pending updates` describe the final data structure.
- `Checksum` makes the final state observable and helps prevent dead-code
  elimination.

Close other applications, use the same machine and power plan, and report the
median of at least five Release runs. Keep the input, compiler, build flags,
buffer size, and CPU configuration unchanged when comparing revisions.
