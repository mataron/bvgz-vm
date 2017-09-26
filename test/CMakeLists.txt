include_directories(.)
include_directories(../src)

include (CTest)
enable_testing()

add_subdirectory(util)

add_definitions(-DPROJECT_ROOT=${PROJECT_SOURCE_DIR})


add_executable(test_parser test_parser.c)
target_link_libraries(test_parser utils vm bytecode)
add_test(test_parser test_parser)
