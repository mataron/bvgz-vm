include_directories(.)
include_directories(../src)

include (CTest)
enable_testing()

add_subdirectory(util)

add_definitions(-DPROJECT_ROOT=${PROJECT_SOURCE_DIR})


add_executable(test_parser test_parser.c)
target_link_libraries(test_parser vm bytecode utils)
add_test(test_parser test_parser)

add_executable(test_bytecode_gen test_bytecode_gen.c)
target_link_libraries(test_bytecode_gen vm bytecode utils)
add_test(test_bytecode_gen test_bytecode_gen)

add_executable(test_bytecode_decode test_bytecode_decode.c)
target_link_libraries(test_bytecode_decode vm bytecode utils)
add_test(test_bytecode_decode test_bytecode_decode)

add_executable(test_vm test_vm.c)
target_link_libraries(test_vm vm bytecode utils)
add_test(test_vm test_vm)
