# Optimized Price Point Order Book

This project implements an in-memory C++17 order book. It loads an initial
snapshot, applies Binance-style depth updates, and maintains the best bid and
best ask for each symbol.

The solution is designed for large books and many symbols. It uses fixed-point
integers for prices and quantities and processes updates in batches to reduce
the cost of changing large sorted vectors.

## Solution Strategy

Each symbol is represented by a `Book` with two independent sides:

- Bids are sorted by price in descending order.
- Asks are sorted by price in ascending order.

The initial snapshot is loaded into vectors and sorted once. Incoming updates
are not inserted directly into the main sorted vectors. They are first appended
to an overflow buffer.

When the buffer reaches its configured limit, or when all updates have been
read, `compact()`:

1. Stable-sorts the buffered updates.
2. Appends them to the main vector.
3. Merges the old levels and new updates.
4. Groups entries with the same price.
5. Keeps the latest value for each price.
6. Removes levels whose latest quantity is zero.

This avoids moving a large part of the main vector for every individual update.

## Input

The project uses two input types.

### Snapshot

The snapshot contains a symbol, bids, and asks:

```json
{
  "symbol": "BNBBTC",
  "bids": [
    ["0.002398500", "12.750000000"],
    ["0.002397000", "8.420000000"]
  ],
  "asks": [
    ["0.002401500", "11.920000000"],
    ["0.002403000", "6.540000000"]
  ]
}
```

Each level is represented by a price string and a quantity string.

### Updates

Each update event is stored on a separate line:

```json
{"e":"depthUpdate","s":"BNBBTC","b":[["0.0024","10"]],"a":[["0.0026","100"]]}
```

The fields used by the reader are:

- `s`: Symbol.
- `b`: Bid updates.
- `a`: Ask updates.

A quantity greater than zero adds a new level or replaces the quantity at an
existing price. A quantity of zero removes the level at that price.

The sample input files are stored in `input_data/`. The application currently
selects the snapshot and update paths in `main.cpp`, so these paths must point
to the files on the machine where the program is executed.

## Architecture

image.png

The project is divided into the following modules:

- `utils`: Fixed-point conversion and pair extraction.
- `data_types`: Price levels, book sides, books, and symbol storage.
- `engine`: Snapshot and update processing.
- `benchmarks`: Performance and memory benchmarks.
- `test`: Unit tests, boundary tests, and input-error tests.

### Processing Flow

The snapshot flow is:

```text
Snapshot file -> SnapshotReader -> Engine -> BookContainer -> Book
```

The update flow is:

```text
Updates file -> UpdatesReader -> Engine -> Book -> overflow buffer -> compact
```

## Data Structures

### PriceLevel

`PriceLevel` contains:

```cpp
std::uint64_t price;
std::uint64_t quantity;
```

Prices and quantities use fixed-point integer values instead of floating-point
numbers. The scale is \(10^9\), which supports nine decimal places.

### PriceLevelContainer

Each side of a book uses a `PriceLevelContainer` with:

- A sorted `std::vector<PriceLevel>` containing active levels.
- An overflow `std::vector<PriceLevel>` containing pending updates.
- An order setting for ascending or descending prices.
- A configurable buffer threshold.

The default update-buffer threshold is 131,072.

### Book

A `Book` owns:

- One descending bid container.
- One ascending ask container.

The first active bid is the best bid. The first active ask is the best ask.

### BookContainer

`BookContainer` stores:

- Books in a contiguous `std::vector<Book>`.
- Symbol-to-index mappings in an `std::unordered_map`.

The map provides average constant-time symbol lookup. The vector provides
compact storage and good iteration locality.

### Engine Cache

While processing updates, `Engine` caches symbol-to-book pointers. This avoids
repeating the full `BookContainer` lookup for every event of a symbol. The cache
is cleared after pending updates are compacted.

## Algorithms

### Fixed-Point Conversion

Decimal strings are converted to unsigned 64-bit integers using a multiplier of
\(10^9\). This avoids floating-point comparison and rounding problems in the
order book.

### Snapshot Loading

For each book side:

1. Parse all price and quantity pairs.
2. Move them into the main vector.
3. Stable-sort them in the required order.
4. Group duplicate prices.
5. Keep the last value for each price.
6. Remove zero-quantity levels.

Snapshot sorting has \(O(n \log n)\) time complexity.

### Update Processing

Each update is appended to the overflow buffer in amortized \(O(1)\) time.
When compaction is required:

1. The buffer is stable-sorted in \(O(k \log k)\).
2. The sorted buffer is merged with the main data.
3. Equal prices are resolved in one linear pass.

Here, \(n\) is the number of active levels and \(k\) is the number of buffered
updates. The merge and duplicate-removal work is linear in the combined data
size.

Stable ordering is important because the latest update for the same price must
win.

### Deletion

A zero-quantity update is kept until compaction. If it is the latest update for
its price, the corresponding level is not written back to the active vector.

### Best Price

Because each side remains sorted, the best price is available from the first
element in \(O(1)\) time.

## Assumptions

- The program processes non-negative prices and quantities.
- Input numbers have no more than nine meaningful decimal places.
- Snapshot input contains a `symbol`, `bids`, and `asks`.
- Each update event is stored on one line.
- Every update symbol already has a loaded book.
- A zero quantity means that the price level must be deleted.
- Updates in the files are already in the order in which they should be
  applied.
- The latest snapshot or update entry for the same price is authoritative.
- Live exchange connectivity and snapshot synchronization are outside the
  project scope.
- The current implementation is single-threaded.
- Input files are expected to follow the supported format. The readers are
  optimized for this format rather than being general JSON parsers.
- Fixed-point values fit in `std::uint64_t` after scaling.

## Build and Run

The project requires:

- CMake 3.16 or newer.
- A C++17 compiler.
- Visual Studio 2022 on Windows, or a compatible GCC/Clang toolchain.

On Windows:

```powershell
.\build.bat
.\run.bat
```

The Release application is created at:

```text
build\bin\Release\order_book_app.exe
```

## Tests

Run the complete test suite with:

```powershell
.\test\run_tests.bat
```

The suite covers the main data structures, fixed-point utilities, snapshot and
update processing, boundary cases, invalid order values, missing files, missing
symbols, and support for 2,000 books.

## Benchmarks

Run the generated data-structure benchmarks with:

```powershell
.\benchmarks\run_benchmarks.bat
```

Run the end-to-end file benchmark with:

```powershell
.\benchmarks\run_file_benchmark.bat input_data\lite_snapshot.txt input_data\lite_updates.txt
```

The reports are written to:

```text
benchmark_logs\performance.txt
benchmark_logs\file_performance.txt
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file
for details.