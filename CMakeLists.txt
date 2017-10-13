cmake_minimum_required(VERSION 3.6)

project(bvgz-vm)
add_definitions(-Wall -std=c99)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -ggdb -O0")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBVGZ_IMG_GEN_DIR=/tmp/bvgz/")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBVGZ_VM_EXEC=/usr/local/bin/bvgz")

add_subdirectory(src)
add_subdirectory(test)

enable_testing()
