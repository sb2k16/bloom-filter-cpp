#pragma once

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <limits>

namespace bloomfilter {

// Type aliases
using size_t = std::size_t;
using uint8_t = std::uint8_t;
using uint32_t = std::uint32_t;
using uint64_t = std::uint64_t;

// Constants
constexpr double DEFAULT_FALSE_POSITIVE_RATE = 0.01;  // 1%
constexpr size_t MIN_BIT_ARRAY_SIZE = 64;
constexpr size_t MAX_BIT_ARRAY_SIZE = std::numeric_limits<size_t>::max() / 8;
constexpr size_t MIN_HASH_FUNCTIONS = 1;
constexpr size_t MAX_HASH_FUNCTIONS = 32;

// Optimal parameter calculation constants
constexpr double LN_2 = 0.6931471805599453;
constexpr double LN_2_SQUARED = 0.4804530139182014;  // (ln(2))^2

/**
 * Calculate optimal bit array size given expected elements and false positive rate
 * Formula: m = -n * ln(p) / (ln(2)^2)
 */
inline size_t calculate_optimal_bit_array_size(size_t expected_elements, double false_positive_rate) {
    if (expected_elements == 0) {
        return MIN_BIT_ARRAY_SIZE;
    }
    if (false_positive_rate <= 0.0 || false_positive_rate >= 1.0) {
        false_positive_rate = DEFAULT_FALSE_POSITIVE_RATE;
    }
    
    double m = -static_cast<double>(expected_elements) * std::log(false_positive_rate) / LN_2_SQUARED;
    size_t result = static_cast<size_t>(std::ceil(m));
    return std::max(result, MIN_BIT_ARRAY_SIZE);
}

/**
 * Calculate optimal number of hash functions given bit array size and expected elements
 * Formula: k = (m/n) * ln(2)
 */
inline size_t calculate_optimal_hash_count(size_t bit_array_size, size_t expected_elements) {
    if (expected_elements == 0) {
        return 1;
    }
    
    double k = (static_cast<double>(bit_array_size) / static_cast<double>(expected_elements)) * LN_2;
    size_t result = static_cast<size_t>(std::round(k));
    return std::max(MIN_HASH_FUNCTIONS, std::min(result, MAX_HASH_FUNCTIONS));
}

/**
 * Estimate false positive rate given current parameters
 * Formula: (1 - e^(-kn/m))^k
 */
inline double estimate_false_positive_rate(size_t bit_array_size, size_t hash_count, size_t inserted_elements) {
    if (bit_array_size == 0 || hash_count == 0) {
        return 1.0;
    }
    if (inserted_elements == 0) {
        return 0.0;
    }
    
    double exponent = -static_cast<double>(hash_count) * static_cast<double>(inserted_elements) / static_cast<double>(bit_array_size);
    double base = 1.0 - std::exp(exponent);
    return std::pow(base, static_cast<double>(hash_count));
}

} // namespace bloomfilter

