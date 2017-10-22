include_directories(.)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(util)
add_subdirectory(instns)
add_subdirectory(syscalls)
add_subdirectory(debug)

add_library(vm STATIC
    vm.c bytecode.c vm_exec.c timers.c
    vm_debug.c
)

add_library(bytecode STATIC
    parser.c bytecode_gen.c
)

add_executable(bvgz bvgz.c)

target_link_libraries(bvgz -Wl,--start-group
    syscalls instns vm_debug vm utils
    -Wl,--end-group)
target_link_libraries(bvgz readline)

add_executable(basm basm_app.c)

target_link_libraries(basm -Wl,--start-group
    syscalls instns bytecode vm utils
    -Wl,--end-group)

add_library(bvgzvm SHARED vmlib.c)
target_link_libraries(bvgzvm -Wl,--start-group
    syscalls instns bytecode vm utils
    -Wl,--end-group)
