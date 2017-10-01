include_directories(.)

add_subdirectory(util)

add_library(vm STATIC
    vm.c instn.c instn_defs.c bytecode.c instn_impl_misc.c
    instn_impl_arith.c instn_impl_bitw.c instn_impl_rel.c
    instn_impl_logc.c instn_impl_ctrl.c vm_exec.c
    instn_impl_sys.c syscall.c syscall_misc.c
)

add_library(bytecode STATIC
    parser.c bytecode_gen.c
)

add_executable(bvgz bvgz.c)

target_link_libraries(bvgz
    vm utils
)

add_executable(basm basm_app.c)

target_link_libraries(basm
    bytecode vm utils
)
