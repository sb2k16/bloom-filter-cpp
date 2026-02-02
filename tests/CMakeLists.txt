cmake_minimum_required(VERSION 3.13)

# Find Google Test
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # If GTest is not found, download it
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Test executable
add_executable(test_bloom_filter test_bloom_filter.cpp)
target_link_libraries(test_bloom_filter GTest::gtest GTest::gtest_main GTest::gmock)
target_include_directories(test_bloom_filter PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Register tests
include(GoogleTest)
gtest_discover_tests(test_bloom_filter)

