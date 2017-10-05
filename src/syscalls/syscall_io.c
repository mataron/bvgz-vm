#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "vm.h"
#include "syscall_io.h"
#include "bytecode.h"


#define FD_ALLOC_STEP   5


uint64_t alloc_fd(vm_t* vm)
{
    uint32_t begin_fd = 0;
    if (vm->io.used_fds == vm->io.n_fds)
    {
        vm->io.fds = realloc(vm->io.fds,
            sizeof(vm_fd_t) * (vm->io.n_fds + FD_ALLOC_STEP));
        memset(vm->io.fds + vm->io.n_fds, 0,
            sizeof(vm_fd_t) * FD_ALLOC_STEP);
        begin_fd = vm->io.n_fds;
        vm->io.n_fds += FD_ALLOC_STEP;
    }

    for (uint32_t i = begin_fd; i < vm->io.n_fds; i++)
    {
        if (!vm->io.fds[i].used)
        {
            vm->io.fds[i].used = 1;
            return i;
        }
    }

    // unreachable!!!
    abort();
    return (uint64_t)-1;
}


void dealloc_fd(uint64_t fd, vm_t* vm)
{
    vm->io.fds[fd].used = 0;
    vm->io.used_fds--;
}


int sys_close(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        return -1;
    }

    uint32_t fd = FD_HANDLE_TO_IDX(args[0]);
    if (fd > vm->io.n_fds || !vm->io.fds[fd].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return 0;
    }

    *ret = 0;
    int close_fd = vm->io.fds[fd].fd;

    if (close(close_fd) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        // fall through:
    }

    dealloc_fd(fd, vm);
    return 0;
}


int sys_read(vm_t* vm, uint32_t argv, uint32_t retv)
{

    return 0;
}


int sys_write(vm_t* vm, uint32_t argv, uint32_t retv)
{
    return 0;
}
