include_directories(.)

add_library(syscalls STATIC
    syscall.c syscall_misc.c
)
