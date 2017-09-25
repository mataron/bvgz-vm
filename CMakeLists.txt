cmake_minimum_required(VERSION 3.9)

project(bvgz-vm)
add_definitions(-Wall -std=c99)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -ggdb -O0")

add_subdirectory(src)
add_subdirectory(test)

enable_testing()
