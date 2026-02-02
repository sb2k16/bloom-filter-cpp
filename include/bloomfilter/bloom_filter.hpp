#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "types.hpp"
#include "hash_functions.hpp"

namespace bloomfilter {

/**
 * Standard Bloom Filter implementation
 * 
 * A space-efficient probabilistic data structure that tests membership in a set.
 * - No false negatives: If it says "not in set", you can trust it
 * - Possible false positives: May say "in set" when element was never inserted
 * 
 * @tparam HashType Type for hash values (typically uint64_t)
 */
template<typename HashType = uint64_t>
class BloomFilter {
public:
    /**
     * Construct a Bloom Filter with specified capacity and false positive rate
     * 
     * @param expected_elements Expected number of elements to insert
     * @param false_positive_rate Desired false positive rate (0.0 to 1.0)
     */
    explicit BloomFilter(size_t expected_elements, double false_positive_rate = DEFAULT_FALSE_POSITIVE_RATE)
        : bit_array_size_(0)  // Will be set in constructor body
        , bit_array_bytes_(0)
        , hash_count_(0)
        , expected_elements_(expected_elements)
        , false_positive_rate_(false_positive_rate)
        , inserted_count_(0)
        , hasher_(1)  // Temporary, will be reassigned
    {
        if (expected_elements == 0) {
            throw std::invalid_argument("Expected elements must be greater than 0");
        }
        if (false_positive_rate <= 0.0 || false_positive_rate >= 1.0) {
            throw std::invalid_argument("False positive rate must be between 0 and 1");
        }
        
        // Calculate optimal parameters
        bit_array_size_ = calculate_optimal_bit_array_size(expected_elements, false_positive_rate);
        hash_count_ = calculate_optimal_hash_count(bit_array_size_, expected_elements);
        
        // Initialize bit array (using bytes, 8 bits per byte)
        bit_array_bytes_ = (bit_array_size_ + 7) / 8;
        bit_array_.resize(bit_array_bytes_, 0);
        
        // Initialize double hasher (must be done after bit_array_size_ is set)
        hasher_ = DoubleHasher(bit_array_size_);
    }
    
    /**
     * Construct a Bloom Filter with explicit parameters
     * 
     * @param bit_array_size Size of the bit array
     * @param hash_count Number of hash functions to use
     * @param expected_elements Expected number of elements (for statistics)
     */
    BloomFilter(size_t bit_array_size, size_t hash_count, size_t expected_elements)
        : bit_array_size_(bit_array_size)
        , bit_array_bytes_((bit_array_size + 7) / 8)
        , hash_count_(hash_count)
        , expected_elements_(expected_elements)
        , false_positive_rate_(estimate_false_positive_rate(bit_array_size, hash_count, expected_elements))
        , inserted_count_(0)
        , hasher_(bit_array_size)
    {
        if (bit_array_size == 0) {
            throw std::invalid_argument("Bit array size must be greater than 0");
        }
        if (hash_count == 0 || hash_count > MAX_HASH_FUNCTIONS) {
            throw std::invalid_argument("Hash count must be between 1 and MAX_HASH_FUNCTIONS");
        }
        
        bit_array_.resize(bit_array_bytes_, 0);
    }
    
    /**
     * Insert an element into the Bloom Filter
     * 
     * @param data Pointer to the data to insert
     * @param size Size of the data in bytes
     */
    void insert(const void* data, size_t size) {
        if (data == nullptr || size == 0) {
            return;
        }
        
        // Generate all hash values
        std::vector<size_t> hashes(hash_count_);
        hasher_.get_all_hashes(data, size, hash_count_, hashes.data());
        
        // Set all corresponding bits
        for (size_t hash : hashes) {
            set_bit(hash);
        }
        
        inserted_count_++;
    }
    
    /**
     * Insert a string into the Bloom Filter
     */
    void insert(const std::string& str) {
        insert(str.data(), str.size());
    }
    
    /**
     * Check if an element might be in the Bloom Filter
     * 
     * @param data Pointer to the data to check
     * @param size Size of the data in bytes
     * @return true if element might be in set (could be false positive)
     *         false if element is definitely not in set (no false negatives)
     */
    bool contains(const void* data, size_t size) const {
        if (data == nullptr || size == 0) {
            return false;
        }
        
        // Generate all hash values
        std::vector<size_t> hashes(hash_count_);
        hasher_.get_all_hashes(data, size, hash_count_, hashes.data());
        
        // Check if all corresponding bits are set
        for (size_t hash : hashes) {
            if (!get_bit(hash)) {
                return false;  // At least one bit is 0, definitely not in set
            }
        }
        
        return true;  // All bits are 1, might be in set (could be false positive)
    }
    
    /**
     * Check if a string might be in the Bloom Filter
     */
    bool contains(const std::string& str) const {
        return contains(str.data(), str.size());
    }
    
    /**
     * Get the number of elements inserted
     */
    size_t size() const {
        return inserted_count_;
    }
    
    /**
     * Get the expected capacity
     */
    size_t capacity() const {
        return expected_elements_;
    }
    
    /**
     * Get the target false positive rate
     */
    double false_positive_rate() const {
        return false_positive_rate_;
    }
    
    /**
     * Get the estimated current false positive rate
     * This increases as more elements are inserted
     */
    double estimated_false_positive_rate() const {
        return estimate_false_positive_rate(bit_array_size_, hash_count_, inserted_count_);
    }
    
    /**
     * Get the size of the bit array
     */
    size_t bit_array_size() const {
        return bit_array_size_;
    }
    
    /**
     * Get the number of hash functions used
     */
    size_t hash_count() const {
        return hash_count_;
    }
    
    /**
     * Get memory usage in bytes
     */
    size_t memory_usage() const {
        return bit_array_bytes_ + sizeof(*this);
    }
    
    /**
     * Clear the Bloom Filter (reset all bits to 0)
     */
    void clear() {
        std::fill(bit_array_.begin(), bit_array_.end(), 0);
        inserted_count_ = 0;
    }
    
    /**
     * Get the number of bits set to 1 (for statistics)
     */
    size_t count_set_bits() const {
        size_t count = 0;
        size_t full_bytes = bit_array_size_ / 8;
        
        // Count bits in full bytes
        for (size_t i = 0; i < full_bytes; ++i) {
            uint8_t byte = bit_array_[i];
            // Count set bits in byte using Brian Kernighan's algorithm
            while (byte) {
                byte &= byte - 1;
                count++;
            }
        }
        
        // Count bits in partial last byte
        size_t extra_bits = bit_array_size_ % 8;
        if (extra_bits > 0 && full_bytes < bit_array_.size()) {
            uint8_t last_byte = bit_array_[full_bytes];
            // Only count bits within bit_array_size_
            uint8_t mask = (1 << extra_bits) - 1;
            last_byte &= mask;
            while (last_byte) {
                last_byte &= last_byte - 1;
                count++;
            }
        }
        
        return count;
    }

private:
    size_t bit_array_size_;
    size_t bit_array_bytes_;
    size_t hash_count_;
    size_t expected_elements_;
    double false_positive_rate_;
    size_t inserted_count_;
    
    std::vector<uint8_t> bit_array_;
    mutable DoubleHasher hasher_;
    
    /**
     * Set a bit at the given position
     */
    void set_bit(size_t position) {
        if (position >= bit_array_size_) {
            return;  // Safety check
        }
        size_t byte_index = position / 8;
        size_t bit_index = position % 8;
        bit_array_[byte_index] |= (1 << bit_index);
    }
    
    /**
     * Get the value of a bit at the given position
     */
    bool get_bit(size_t position) const {
        if (position >= bit_array_size_) {
            return false;  // Safety check
        }
        size_t byte_index = position / 8;
        size_t bit_index = position % 8;
        return (bit_array_[byte_index] & (1 << bit_index)) != 0;
    }
};

} // namespace bloomfilter

