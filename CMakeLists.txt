cmake_minimum_required(VERSION 3.22)
project(lipp)

#add_compile_options(--std=c++11 -O3 -Wall)
add_definitions(-std=c++11 -g)
include_directories(src/core)

add_executable(example src/examples/example.cpp)
add_executable(example_without_fmcd src/examples/example_without_fmcd.cpp)
add_executable(example_bulk_load src/examples/example_bulk_load.cpp)
add_executable(benchmark src/benchmark/main.cpp)
#
#add_executable(testFMCD src/examples/test_insert_blank.cpp)
#add_executable(test_insert_blank src/examples/testFMCD.cpp)
