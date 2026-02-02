#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include "types.hpp"

namespace bloomfilter {

/**
 * MurmurHash3 64-bit hash function
 * Fast, well-distributed hash function suitable for Bloom Filters
 */
class MurmurHash3 {
public:
    explicit MurmurHash3(uint64_t seed = 0) : seed_(seed) {}
    
    uint64_t operator()(const void* data, size_t size) const {
        return hash(data, size, seed_);
    }
    
    uint64_t hash(const void* data, size_t size, uint64_t seed) const {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        const uint64_t* blocks = reinterpret_cast<const uint64_t*>(bytes);
        const size_t nblocks = size / 8;
        
        uint64_t h1 = seed;
        const uint64_t c1 = 0x87c37b91114253d5ULL;
        const uint64_t c2 = 0x4cf5ad432745937fULL;
        
        // Body
        for (size_t i = 0; i < nblocks; ++i) {
            uint64_t k1 = blocks[i];
            
            k1 *= c1;
            k1 = rotl64(k1, 31);
            k1 *= c2;
            
            h1 ^= k1;
            h1 = rotl64(h1, 27);
            h1 = h1 * 5 + 0x52dce729;
        }
        
        // Tail
        const uint8_t* tail = bytes + nblocks * 8;
        uint64_t k1 = 0;
        
        switch (size & 7) {
            case 7: k1 ^= static_cast<uint64_t>(tail[6]) << 48;
            case 6: k1 ^= static_cast<uint64_t>(tail[5]) << 40;
            case 5: k1 ^= static_cast<uint64_t>(tail[4]) << 32;
            case 4: k1 ^= static_cast<uint64_t>(tail[3]) << 24;
            case 3: k1 ^= static_cast<uint64_t>(tail[2]) << 16;
            case 2: k1 ^= static_cast<uint64_t>(tail[1]) << 8;
            case 1: k1 ^= static_cast<uint64_t>(tail[0]);
                    k1 *= c1;
                    k1 = rotl64(k1, 31);
                    k1 *= c2;
                    h1 ^= k1;
        }
        
        // Finalization
        h1 ^= size;
        h1 = fmix64(h1);
        
        return h1;
    }

private:
    uint64_t seed_;
    
    static inline uint64_t rotl64(uint64_t x, int r) {
        return (x << r) | (x >> (64 - r));
    }
    
    static inline uint64_t fmix64(uint64_t k) {
        k ^= k >> 33;
        k *= 0xff51afd7ed558ccdULL;
        k ^= k >> 33;
        k *= 0xc4ceb9fe1a85ec53ULL;
        k ^= k >> 33;
        return k;
    }
};

/**
 * FNV-1a hash function (64-bit variant)
 * Simple, fast hash function
 */
class FNV1a {
public:
    explicit FNV1a(uint64_t seed = 0) : seed_(seed) {}
    
    uint64_t operator()(const void* data, size_t size) const {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        uint64_t hash = 0xcbf29ce484222325ULL ^ seed_;  // FNV offset basis
        
        for (size_t i = 0; i < size; ++i) {
            hash ^= bytes[i];
            hash *= 0x100000001b3ULL;  // FNV prime
        }
        
        return hash;
    }

private:
    uint64_t seed_;
};

/**
 * Double hashing technique for generating k hash values
 * Uses two base hash functions to generate k independent-looking hash values
 * This is more efficient than using k independent hash functions
 */
class DoubleHasher {
public:
    DoubleHasher(size_t bit_array_size, uint64_t seed1 = 0, uint64_t seed2 = 0x1234567890abcdefULL)
        : bit_array_size_(bit_array_size), hash1_(seed1), hash2_(seed2) {}
    
    /**
     * Generate the i-th hash value for the given data
     * Formula: h_i(x) = (h1(x) + i * h2(x)) mod m
     */
    size_t hash(const void* data, size_t size, size_t hash_index) const {
        uint64_t h1 = hash1_(data, size);
        uint64_t h2 = hash2_(data, size);
        
        // Ensure h2 is odd for better distribution
        if (h2 % 2 == 0) {
            h2 += 1;
        }
        
        uint64_t result = (h1 + hash_index * h2) % bit_array_size_;
        return static_cast<size_t>(result);
    }
    
    /**
     * Generate all k hash values for the given data
     */
    void get_all_hashes(const void* data, size_t size, size_t k, size_t* hashes) const {
        uint64_t h1 = hash1_(data, size);
        uint64_t h2 = hash2_(data, size);
        
        // Ensure h2 is odd for better distribution
        if (h2 % 2 == 0) {
            h2 += 1;
        }
        
        for (size_t i = 0; i < k; ++i) {
            uint64_t result = (h1 + i * h2) % bit_array_size_;
            hashes[i] = static_cast<size_t>(result);
        }
    }

private:
    size_t bit_array_size_;
    MurmurHash3 hash1_;
    MurmurHash3 hash2_;
};

} // namespace bloomfilter

