include_directories(.)
include_directories(../src)

include (CTest)
enable_testing()

add_subdirectory(util)

add_definitions(-DPROJECT_ROOT=${PROJECT_SOURCE_DIR})


add_library(testing STATIC
    tests.c
)


add_executable(test_parser test_parser.c)
target_link_libraries(test_parser -Wl,--start-group
    syscalls instns vm bytecode utils
    -Wl,--end-group)
add_test(test_parser test_parser)

add_executable(test_bytecode_gen test_bytecode_gen.c)
target_link_libraries(test_bytecode_gen -Wl,--start-group
    syscalls instns vm bytecode utils
    -Wl,--end-group)
add_test(test_bytecode_gen test_bytecode_gen)

add_executable(test_bytecode_decode test_bytecode_decode.c)
target_link_libraries(test_bytecode_decode -Wl,--start-group
    syscalls instns vm bytecode utils
    -Wl,--end-group)
add_test(test_bytecode_decode test_bytecode_decode)

add_executable(test_vm test_vm.c)
target_link_libraries(test_vm -Wl,--start-group
    testing syscalls instns vm bytecode utils
    -Wl,--end-group)
add_test(test_vm test_vm)

add_executable(test_vm_sys test_vm_sys.c)
target_link_libraries(test_vm_sys -Wl,--start-group
    testing syscalls instns vm bytecode utils
    -Wl,--end-group)
add_test(test_vm_sys test_vm_sys)

add_executable(test_vm_sys_fs test_vm_sys_fs.c)
target_link_libraries(test_vm_sys_fs -Wl,--start-group
    testing syscalls instns vm bytecode utils
    -Wl,--end-group)
add_test(test_vm_sys_fs test_vm_sys_fs)

add_executable(test_vm_sys_net test_vm_sys_net.c)
target_link_libraries(test_vm_sys_net -Wl,--start-group
    testing syscalls instns vm bytecode utils
    -Wl,--end-group)
target_link_libraries(test_vm_sys_net pthread)
add_test(test_vm_sys_net test_vm_sys_net)

add_executable(test_vm_sys_mem test_vm_sys_mem.c)
target_link_libraries(test_vm_sys_mem -Wl,--start-group
    testing syscalls instns vm bytecode utils
    -Wl,--end-group)
add_test(test_vm_sys_mem test_vm_sys_mem)
