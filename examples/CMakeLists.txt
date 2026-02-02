cmake_minimum_required(VERSION 3.13)

# Basic usage example
add_executable(basic_usage basic_usage.cpp)
target_include_directories(basic_usage PRIVATE ${CMAKE_SOURCE_DIR}/include)

