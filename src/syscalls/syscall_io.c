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


uint32_t fire_io_events(struct _vm_t* vm)
{
    return 0;
}


void sys_close(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(args[0]);
    if (fd_idx > vm->io.n_fds || !vm->io.fds[fd_idx].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return;
    }

    *ret = 0;
    int fd = vm->io.fds[fd_idx].fd;

    if (close(fd) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        // fall through:
    }

    dealloc_fd(fd_idx, vm);
}


static uint64_t* get_io_task_params(vm_t* vm, uint32_t argv,
    uint32_t retv, uint32_t* ptr, uint32_t* len, uint32_t* f_ptr,
    vm_fd_t** fd)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 4 * 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return NULL;
    }

    if (!FITS_IN_32Bit(args[0]) || !FITS_IN_32Bit(args[1]) ||
        !FITS_IN_32Bit(args[2]) || !FITS_IN_32Bit(args[3]))
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return NULL;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(args[0]);
    if (fd_idx > vm->io.n_fds || !vm->io.fds[fd_idx].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return NULL;
    }
    *fd = vm->io.fds + fd_idx;

    *ptr = args[1];
    *len = args[2];

    if (!deref(*ptr, *len, vm))
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return NULL;
    }

    *f_ptr = args[3];
    if (*f_ptr > vm->codesz)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return NULL;
    }

    return ret;
}


void sys_read(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint32_t ptr;
    uint32_t len;
    uint32_t f_ptr;
    vm_fd_t* fd;

    uint64_t* ret = get_io_task_params(vm, argv, retv,
        &ptr, &len, &f_ptr, &fd);
    if (!ret)
    {
        return;
    }

    if (fd->n_read_bufs + 1 > fd->alloc_read_bufs)
    {
        fd->read_bufs = realloc(fd->read_bufs,
            sizeof(io_mem_t) * (fd->alloc_read_bufs + IO_BUF_ALLOC));
        fd->alloc_read_bufs += IO_BUF_ALLOC;
    }

    io_mem_t* buf = fd->read_bufs + fd->n_read_bufs;
    fd->n_read_bufs++;

    buf->ptr = ptr;
    buf->len = len;
    buf->callback = f_ptr;

    *ret = 0;
}


void sys_write(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint32_t ptr;
    uint32_t len;
    uint32_t f_ptr;
    vm_fd_t* fd;

    uint64_t* ret = get_io_task_params(vm, argv, retv,
        &ptr, &len, &f_ptr, &fd);
    if (!ret)
    {
        return;
    }

    if (fd->n_write_bufs + 1 > fd->alloc_write_bufs)
    {
        fd->write_bufs = realloc(fd->write_bufs,
            sizeof(io_mem_t) * (fd->alloc_write_bufs + IO_BUF_ALLOC));
        fd->alloc_write_bufs += IO_BUF_ALLOC;
    }

    io_mem_t* buf = fd->write_bufs + fd->n_write_bufs;
    fd->n_write_bufs++;

    buf->ptr = ptr;
    buf->len = len;
    buf->callback = f_ptr;

    *ret = 0;
}
