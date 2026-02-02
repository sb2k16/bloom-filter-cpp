#include <iostream>
#include <cstring>
#include <vector>
#include <bloomfilter/bloom_filter.hpp>
#include <bloomfilter/types.hpp>

using namespace bloomfilter;

int main() {
    std::cout << "=== Bloom Filter Basic Usage Example ===\n\n";
    
    // Create a Bloom Filter with capacity of 1000 elements and 1% false positive rate
    BloomFilter<> filter(1000, 0.01);
    
    std::cout << "Bloom Filter Configuration:\n";
    std::cout << "  Capacity: " << filter.capacity() << " elements\n";
    std::cout << "  Target False Positive Rate: " << (filter.false_positive_rate() * 100) << "%\n";
    std::cout << "  Bit Array Size: " << filter.bit_array_size() << " bits\n";
    std::cout << "  Hash Functions: " << filter.hash_count() << "\n";
    std::cout << "  Memory Usage: " << (filter.memory_usage() / 1024.0) << " KB\n\n";
    
    // Insert some elements
    std::cout << "Inserting elements...\n";
    filter.insert("apple");
    filter.insert("banana");
    filter.insert("cherry");
    filter.insert("date");
    filter.insert("elderberry");
    
    std::cout << "  Inserted " << filter.size() << " elements\n\n";
    
    // Query for existing elements
    std::cout << "Querying for existing elements:\n";
    std::vector<std::string> existing = {"apple", "banana", "cherry"};
    for (const auto& elem : existing) {
        bool found = filter.contains(elem);
        std::cout << "  '" << elem << "': " << (found ? "MIGHT BE in set" : "NOT in set") << "\n";
    }
    
    // Query for non-existing elements
    std::cout << "\nQuerying for non-existing elements:\n";
    std::vector<std::string> non_existing = {"grape", "kiwi", "mango"};
    for (const auto& elem : non_existing) {
        bool found = filter.contains(elem);
        std::cout << "  '" << elem << "': " << (found ? "MIGHT BE in set (false positive!)" : "NOT in set") << "\n";
    }
    
    // Show statistics
    std::cout << "\nStatistics:\n";
    std::cout << "  Elements inserted: " << filter.size() << "\n";
    std::cout << "  Bits set: " << filter.count_set_bits() << "\n";
    std::cout << "  Estimated false positive rate: " << (filter.estimated_false_positive_rate() * 100) << "%\n";
    
    // Demonstrate no false negatives
    std::cout << "\n=== No False Negatives Guarantee ===\n";
    std::cout << "All inserted elements are found (no false negatives):\n";
    std::vector<std::string> all_inserted = {"apple", "banana", "cherry", "date", "elderberry"};
    bool all_found = true;
    for (const auto& elem : all_inserted) {
        if (!filter.contains(elem)) {
            std::cout << "  ERROR: '" << elem << "' not found!\n";
            all_found = false;
        }
    }
    if (all_found) {
        std::cout << "  ✓ All inserted elements found correctly\n";
    }
    
    // Demonstrate inserting different data types
    std::cout << "\n=== Inserting Different Data Types ===\n";
    
    // Insert integer
    int number = 42;
    filter.insert(&number, sizeof(number));
    std::cout << "  Inserted integer: " << number << "\n";
    std::cout << "  Contains integer: " << (filter.contains(&number, sizeof(number)) ? "Yes" : "No") << "\n";
    
    // Insert raw bytes
    const char* raw_data = "raw_bytes_data";
    filter.insert(raw_data, strlen(raw_data));
    std::cout << "  Inserted raw bytes: " << raw_data << "\n";
    std::cout << "  Contains raw bytes: " << (filter.contains(raw_data, strlen(raw_data)) ? "Yes" : "No") << "\n";
    
    return 0;
}

