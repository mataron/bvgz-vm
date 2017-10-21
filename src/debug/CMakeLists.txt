include_directories(.)

add_library(vm_debug STATIC
    debug.c debug_util.c
    debug_dump.c debug_brk.c debug_exec.c
    debug_procdr.c
)
