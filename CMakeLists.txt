cmake_minimum_required(VERSION 3.13)
project(BloomFilter VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

# Include directories
include_directories(include)

# Source files (if any implementation files)
# Most of the library is header-only, but we can add source files here if needed

# Install headers
install(DIRECTORY include/ DESTINATION include)

# Build tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build benchmarks
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

