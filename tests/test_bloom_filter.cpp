#include <gtest/gtest.h>
#include <bloomfilter/bloom_filter.hpp>
#include <bloomfilter/types.hpp>
#include <string>
#include <vector>
#include <set>
#include <random>

using namespace bloomfilter;

class BloomFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common test setup
    }
};

// Test basic construction
TEST_F(BloomFilterTest, Construction) {
    BloomFilter<> filter(1000, 0.01);
    
    EXPECT_EQ(filter.capacity(), 1000);
    EXPECT_EQ(filter.false_positive_rate(), 0.01);
    EXPECT_EQ(filter.size(), 0);
    EXPECT_GT(filter.bit_array_size(), 0);
    EXPECT_GT(filter.hash_count(), 0);
}

// Test construction with invalid parameters
TEST_F(BloomFilterTest, InvalidConstruction) {
    EXPECT_THROW(BloomFilter<>(0, 0.01), std::invalid_argument);
    EXPECT_THROW(BloomFilter<>(1000, 0.0), std::invalid_argument);
    EXPECT_THROW(BloomFilter<>(1000, 1.0), std::invalid_argument);
    EXPECT_THROW(BloomFilter<>(1000, -0.1), std::invalid_argument);
    EXPECT_THROW(BloomFilter<>(1000, 1.1), std::invalid_argument);
}

// Test insertion and query
TEST_F(BloomFilterTest, InsertAndQuery) {
    BloomFilter<> filter(100, 0.01);
    
    std::string element = "test_element";
    filter.insert(element);
    
    EXPECT_EQ(filter.size(), 1);
    EXPECT_TRUE(filter.contains(element));
}

// Test no false negatives
TEST_F(BloomFilterTest, NoFalseNegatives) {
    BloomFilter<> filter(1000, 0.01);
    
    std::vector<std::string> elements = {
        "apple", "banana", "cherry", "date", "elderberry",
        "fig", "grape", "honeydew", "kiwi", "lemon"
    };
    
    // Insert all elements
    for (const auto& elem : elements) {
        filter.insert(elem);
    }
    
    EXPECT_EQ(filter.size(), elements.size());
    
    // All inserted elements should be found
    for (const auto& elem : elements) {
        EXPECT_TRUE(filter.contains(elem)) << "Element '" << elem << "' should be found";
    }
}

// Test false positives (statistical test)
TEST_F(BloomFilterTest, FalsePositives) {
    const size_t capacity = 1000;
    const double target_fpr = 0.01;
    BloomFilter<> filter(capacity, target_fpr);
    
    // Insert known elements
    std::set<std::string> inserted;
    for (size_t i = 0; i < capacity; ++i) {
        std::string elem = "element_" + std::to_string(i);
        filter.insert(elem);
        inserted.insert(elem);
    }
    
    // Test elements that were NOT inserted
    size_t false_positives = 0;
    size_t tests = 10000;
    
    for (size_t i = capacity; i < capacity + tests; ++i) {
        std::string elem = "element_" + std::to_string(i);
        if (filter.contains(elem)) {
            false_positives++;
        }
    }
    
    double actual_fpr = static_cast<double>(false_positives) / tests;
    double expected_fpr = filter.estimated_false_positive_rate();
    
    // Allow some variance (actual FPR should be close to expected)
    // With 10,000 tests, we expect ~100 false positives for 1% FPR
    // Allow 50-150 range (0.5% to 1.5%)
    EXPECT_GT(actual_fpr, 0.005) << "False positive rate seems too low";
    EXPECT_LT(actual_fpr, 0.02) << "False positive rate seems too high";
    
    // Estimated FPR should be reasonable
    EXPECT_GT(expected_fpr, 0.0);
    EXPECT_LT(expected_fpr, 0.1);
}

// Test empty filter
TEST_F(BloomFilterTest, EmptyFilter) {
    BloomFilter<> filter(100, 0.01);
    
    EXPECT_EQ(filter.size(), 0);
    EXPECT_FALSE(filter.contains("anything"));
    EXPECT_EQ(filter.count_set_bits(), 0);
}

// Test clear operation
TEST_F(BloomFilterTest, Clear) {
    BloomFilter<> filter(100, 0.01);
    
    filter.insert("test1");
    filter.insert("test2");
    EXPECT_EQ(filter.size(), 2);
    EXPECT_TRUE(filter.contains("test1"));
    
    filter.clear();
    EXPECT_EQ(filter.size(), 0);
    EXPECT_FALSE(filter.contains("test1"));
    EXPECT_FALSE(filter.contains("test2"));
    EXPECT_EQ(filter.count_set_bits(), 0);
}

// Test memory usage
TEST_F(BloomFilterTest, MemoryUsage) {
    BloomFilter<> filter(1000, 0.01);
    
    size_t memory = filter.memory_usage();
    EXPECT_GT(memory, 0);
    
    // Memory should be roughly bit_array_size / 8
    EXPECT_GE(memory, filter.bit_array_size() / 8);
}

// Test optimal parameter calculation
TEST_F(BloomFilterTest, OptimalParameters) {
    const size_t expected_elements = 10000;
    const double fpr = 0.01;
    
    BloomFilter<> filter(expected_elements, fpr);
    
    // Verify parameters are reasonable
    size_t optimal_m = calculate_optimal_bit_array_size(expected_elements, fpr);
    size_t optimal_k = calculate_optimal_hash_count(optimal_m, expected_elements);
    
    EXPECT_EQ(filter.bit_array_size(), optimal_m);
    EXPECT_EQ(filter.hash_count(), optimal_k);
}

// Test with different data types
TEST_F(BloomFilterTest, DifferentDataTypes) {
    BloomFilter<> filter(100, 0.01);
    
    // Insert string
    filter.insert("string_data");
    
    // Insert raw bytes
    const char* raw_data = "raw_bytes";
    filter.insert(raw_data, strlen(raw_data));
    
    // Insert integer
    int value = 42;
    filter.insert(&value, sizeof(value));
    
    EXPECT_EQ(filter.size(), 3);
    EXPECT_TRUE(filter.contains("string_data"));
    EXPECT_TRUE(filter.contains(raw_data, strlen(raw_data)));
    EXPECT_TRUE(filter.contains(&value, sizeof(value)));
}

// Test null/empty data handling
TEST_F(BloomFilterTest, NullEmptyData) {
    BloomFilter<> filter(100, 0.01);
    
    size_t initial_size = filter.size();
    
    // Insert null pointer
    filter.insert(nullptr, 0);
    EXPECT_EQ(filter.size(), initial_size);
    
    // Insert empty string
    filter.insert("");
    EXPECT_EQ(filter.size(), initial_size);
    
    // Query null pointer
    EXPECT_FALSE(filter.contains(nullptr, 0));
    
    // Query empty string
    EXPECT_FALSE(filter.contains(""));
}

// Test count_set_bits
TEST_F(BloomFilterTest, CountSetBits) {
    BloomFilter<> filter(100, 0.01);
    
    EXPECT_EQ(filter.count_set_bits(), 0);
    
    filter.insert("test1");
    size_t bits_after_one = filter.count_set_bits();
    EXPECT_GT(bits_after_one, 0);
    EXPECT_LE(bits_after_one, filter.hash_count());
    
    filter.insert("test2");
    size_t bits_after_two = filter.count_set_bits();
    EXPECT_GE(bits_after_two, bits_after_one);
}

// Test estimated false positive rate increases with more insertions
TEST_F(BloomFilterTest, FalsePositiveRateIncreases) {
    BloomFilter<> filter(1000, 0.01);
    
    double initial_fpr = filter.estimated_false_positive_rate();
    EXPECT_EQ(initial_fpr, 0.0);
    
    // Insert half capacity
    for (size_t i = 0; i < 500; ++i) {
        filter.insert("elem_" + std::to_string(i));
    }
    
    double half_fpr = filter.estimated_false_positive_rate();
    EXPECT_GT(half_fpr, initial_fpr);
    
    // Insert full capacity
    for (size_t i = 500; i < 1000; ++i) {
        filter.insert("elem_" + std::to_string(i));
    }
    
    double full_fpr = filter.estimated_false_positive_rate();
    EXPECT_GT(full_fpr, half_fpr);
    EXPECT_LT(full_fpr, 0.1);  // Should still be reasonable
}

// Test explicit parameter construction
TEST_F(BloomFilterTest, ExplicitParameters) {
    size_t bit_array_size = 10000;
    size_t hash_count = 7;
    size_t expected_elements = 1000;
    
    BloomFilter<> filter(bit_array_size, hash_count, expected_elements);
    
    EXPECT_EQ(filter.bit_array_size(), bit_array_size);
    EXPECT_EQ(filter.hash_count(), hash_count);
    EXPECT_EQ(filter.capacity(), expected_elements);
}

// Test hash function independence
TEST_F(BloomFilterTest, HashIndependence) {
    BloomFilter<> filter1(100, 0.01);
    BloomFilter<> filter2(100, 0.01);
    
    // Insert same elements
    for (int i = 0; i < 10; ++i) {
        std::string elem = "elem_" + std::to_string(i);
        filter1.insert(elem);
        filter2.insert(elem);
    }
    
    // Both should find the same elements
    for (int i = 0; i < 10; ++i) {
        std::string elem = "elem_" + std::to_string(i);
        EXPECT_EQ(filter1.contains(elem), filter2.contains(elem));
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

