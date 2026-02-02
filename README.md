# High-Performance Bloom Filter Library

A production-ready, high-performance Bloom Filter library in C++ with space-efficient probabilistic data structures, optimized for caching and deduplication use cases.

## Features

- ✅ **Standard Bloom Filter** - Fast membership testing with configurable false positive rate
- ✅ **Optimal Parameter Calculation** - Automatically calculates optimal bit array size and hash count
- ✅ **High-Quality Hash Functions** - MurmurHash3 and double hashing for excellent distribution
- ✅ **Template-Based Design** - Flexible and type-safe
- ✅ **Memory Efficient** - Uses minimal space (typically ~10 bits per element for 1% false positive rate)
- ✅ **No False Negatives** - If it says "not in set", you can trust it
- 🔄 **Thread Safety** - Coming in Phase 2
- 🔄 **Serialization** - Coming in Phase 3
- 🔄 **Counting & Scalable Variants** - Coming in Phase 4

## Quick Start

### Building the Library

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# Run tests
ctest

# Or run tests directly
./tests/test_bloom_filter
```

### Basic Usage

```cpp
#include <bloomfilter/bloom_filter.hpp>

using namespace bloomfilter;

// Create a Bloom Filter with capacity of 1000 elements and 1% false positive rate
BloomFilter<> filter(1000, 0.01);

// Insert elements
filter.insert("apple");
filter.insert("banana");
filter.insert("cherry");

// Check membership
if (filter.contains("apple")) {
    // "apple" MIGHT be in the set (could be a false positive)
    // But if it returns false, "apple" is DEFINITELY not in the set
}

// Insert different data types
int number = 42;
filter.insert(&number, sizeof(number));

const char* data = "raw_bytes";
filter.insert(data, strlen(data));
```

### Running Examples

```bash
cd build
./examples/basic_usage
```

## API Overview

### Construction

```cpp
// Create with expected capacity and false positive rate
BloomFilter<> filter(expected_elements, false_positive_rate);

// Create with explicit parameters
BloomFilter<> filter(bit_array_size, hash_count, expected_elements);
```

### Operations

```cpp
// Insert elements
filter.insert(data, size);           // Raw bytes
filter.insert(string);                // String

// Query membership
bool might_exist = filter.contains(data, size);
bool might_exist = filter.contains(string);

// Statistics
size_t inserted = filter.size();
size_t capacity = filter.capacity();
double fpr = filter.false_positive_rate();
double estimated_fpr = filter.estimated_false_positive_rate();
size_t memory = filter.memory_usage();

// Clear filter
filter.clear();
```

## How It Works

A Bloom Filter is a probabilistic data structure that:
- **Never has false negatives**: If it says "not in set", the element is definitely not in the set
- **May have false positives**: If it says "might be in set", the element could be a false positive

The false positive rate can be controlled by adjusting the bit array size. See [docs/ALGORITHM.md](docs/ALGORITHM.md) for detailed explanation.

## Performance

- **Insert**: O(k) where k is the number of hash functions (typically 3-10)
- **Query**: O(k) - same as insert
- **Memory**: ~9.6 bits per element for 1% false positive rate
- **Space Efficiency**: Much more space-efficient than hash tables

## Requirements

- C++17 or later
- CMake 3.13 or later
- Google Test (automatically downloaded if not found)

## Project Structure

```
BloomFilter/
├── include/bloomfilter/    # Header files
│   ├── bloom_filter.hpp   # Main Bloom Filter class
│   ├── hash_functions.hpp # Hash function implementations
│   └── types.hpp          # Type definitions and utilities
├── tests/                 # Unit tests
├── examples/              # Usage examples
├── docs/                  # Documentation
└── CMakeLists.txt         # Build configuration
```

## Documentation

- [Algorithm Explanation](docs/ALGORITHM.md) - How Bloom Filters work
- [Implementation Plan](IMPLEMENTATION_PLAN.md) - Development roadmap

## Example Use Cases

1. **Database Query Optimization**: Check if a key exists before expensive database query
2. **Web Caching**: Check if URL is cached before cache lookup
3. **Distributed Systems**: Check if data exists on remote servers before network requests
4. **Deduplication**: Check if data has been seen before
5. **Spell Checkers**: Quick dictionary lookup

## Reducing False Positives

To reduce false positives:
1. **Increase bit array size** - Most effective method
2. **Use optimal hash count** - Automatically calculated
3. **Use better hash functions** - Already using high-quality hash functions
4. **Two-stage verification** - Use Bloom Filter as pre-filter, verify positives definitively

See [docs/ALGORITHM.md](docs/ALGORITHM.md) for detailed strategies.

## License

MIT or Apache 2.0 (to be decided)

## Contributing

This is an active development project. See [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) for the development roadmap.

## Status

**Phase 1: Core Bloom Filter** ✅ Complete
- Basic Bloom Filter class
- Hash function implementations
- Optimal parameter calculation
- Unit tests

**Phase 2: Thread Safety** 🔄 Next
- Atomic operations
- Lock-free operations
- Thread safety tests

**Phase 3: Serialization** 📋 Planned
- Binary serialization
- Versioning support

**Phase 4: Advanced Variants** 📋 Planned
- Counting Bloom Filter
- Scalable Bloom Filter

