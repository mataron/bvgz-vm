include_directories(.)

add_subdirectory(util)

add_library(vm STATIC
    vm.c instn.c instn_defs.c
)

add_library(parser STATIC
    parser.c
)

add_executable(bvgz main.c)

target_link_libraries(bvgz
    vm utils
)
