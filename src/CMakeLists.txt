include_directories(.)

add_subdirectory(util)

add_library(vm STATIC
    vm.c instn.c instn_defs.c bytecode.c instn_impl_misc.c
    instn_impl_arith_bitw.c instn_impl_rel_logc.c
)

add_library(bytecode STATIC
    parser.c bytecode_gen.c
)

add_executable(bvgz main.c)

target_link_libraries(bvgz
    vm utils
)
