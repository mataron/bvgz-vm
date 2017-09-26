include_directories(.)

add_subdirectory(util)

add_library(vm STATIC
    vm.c instn.c instn_defs.c
)

add_library(bytecode STATIC
    parser.c bytecode.c
)

add_executable(bvgz main.c)

target_link_libraries(bvgz
    vm utils
)
