include_directories(.)

add_subdirectory(util)
add_subdirectory(instns)
add_subdirectory(syscalls)

add_library(vm STATIC
    vm.c bytecode.c vm_exec.c
)

add_library(bytecode STATIC
    parser.c bytecode_gen.c
)

add_executable(bvgz bvgz.c)

target_link_libraries(bvgz -Wl,--start-group
    syscalls instns vm utils
    -Wl,--end-group)

add_executable(basm basm_app.c)

target_link_libraries(basm -Wl,--start-group
    syscalls instns bytecode vm utils
    -Wl,--end-group)
