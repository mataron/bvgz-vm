#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

#include "vm.h"
#include "syscall_io.h"
#include "bytecode.h"


#define FD_ALLOC_STEP   5


void destroy_vm_io(vm_t* vm)
{
    if (!vm || !vm->io.fds) return;

    for (uint32_t n = 0; n < vm->io.n_fds; n++)
    {
        if (!vm->io.fds[n].used) continue;

        free(vm->io.fds[n].read_bufs);
        free(vm->io.fds[n].write_bufs);

        close(vm->io.fds[n].fd);
    }
}


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
            vm->io.used_fds++;
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


static uint32_t fire_io_event(vm_t* vm, vm_fd_t* fd, int is_read);
static void fire_read_event(vm_t* vm, vm_fd_t* fd, io_mem_t* evt,
    uint8_t* buf, uint64_t* cb_args);
static void fire_write_event(vm_t* vm, vm_fd_t* fd, io_mem_t* evt,
    uint8_t* buf, uint64_t* cb_args);


uint32_t fire_io_events(vm_t* vm)
{
    struct timespec tm = { 0, 0 };
    fd_set readset, writeset;
    int maxfd = 0;

    if (!vm->io.n_io_bufs)
    {
        return 0;
    }

    for (uint32_t fd_idx = 0; fd_idx < vm->io.n_fds; fd_idx++)
    {
        vm_fd_t* fd = vm->io.fds + fd_idx;
        if (!fd->used) continue;
        if (fd->n_read_bufs)
        {
            FD_SET(fd->fd, &readset);
            if (maxfd < fd->fd) maxfd = fd->fd;
        }
        if (fd->n_write_bufs)
        {
            FD_SET(fd->fd, &writeset);
            if (maxfd < fd->fd) maxfd = fd->fd;
        }
    }

    int ret = pselect(maxfd + 1, &readset, &writeset, NULL, &tm, NULL);
    if (ret <= 0)
    {
        if (ret < 0) vm->error_no = errno;
        return 0;
    }

    uint32_t events = 0;
    for (uint32_t fd_idx = 0; fd_idx < vm->io.n_fds; fd_idx++)
    {
        vm_fd_t* fd = vm->io.fds + fd_idx;
        if (!fd->used) continue;
        if (fd->n_read_bufs && FD_ISSET(fd->fd, &readset))
        {
            events += fire_io_event(vm, fd, 1);
        }
        if (fd->n_write_bufs && FD_ISSET(fd->fd, &writeset))
        {
            events += fire_io_event(vm, fd, 0);
        }
    }

    return events;
}


int has_pending_io_events(vm_t* vm)
{
    return vm->io.n_io_bufs > 0;
}


static uint32_t fire_io_event(vm_t* vm, vm_fd_t* fd, int is_read)
{
    io_mem_t* evt = fd->read_bufs;
    uint64_t* cb_args = (uint64_t*)deref(evt->args, 4 * 8, vm);
    if (!cb_args)
    {
        // not much to do...
        vm->error_no = EFAULT;
        return 0;
    }

    cb_args[0] = FD_IDX_TO_HANDLE(fd - vm->io.fds);
    cb_args[1] = 0; // errno
    cb_args[2] = 0; // bytes read

    uint8_t* buf = deref(evt->ptr, evt->len, vm);
    if (!buf)
    {
        cb_args[1] = EFAULT;
        return 0;
    }

    if (is_read)
    {
        fire_read_event(vm, fd, evt, buf, cb_args);
    }
    else
    {
        fire_write_event(vm, fd, evt, buf, cb_args);
    }

    return 1;
}


static void fire_read_event(vm_t* vm, vm_fd_t* fd, io_mem_t* evt,
    uint8_t* buf, uint64_t* cb_args)
{
    int fd_idx = fd - vm->io.fds;

    ssize_t len = read(fd->fd, buf, evt->len);
    cb_args[2] = len < 0 ? 0 : len;
    cb_args[1] = len < 0 ? errno : 0;

    make_func_procedure(evt->callback, evt->args, 0, vm);

    int rest_bufs = fd->n_read_bufs - fd_idx - 1;
    if (rest_bufs)
    {
        memmove(fd, fd + 1, rest_bufs * sizeof(io_mem_t));
    }

    fd->n_read_bufs--;
    if (fd->n_read_bufs &&
        fd->alloc_read_bufs - fd->n_read_bufs > IO_BUF_ALLOC)
    {
        fd->read_bufs = realloc(fd->read_bufs,
            sizeof(io_mem_t) * (fd->alloc_read_bufs - IO_BUF_ALLOC));
        fd->alloc_read_bufs -= IO_BUF_ALLOC;
    }
    vm->io.n_io_bufs--;
}


static void fire_write_event(vm_t* vm, vm_fd_t* fd, io_mem_t* evt,
    uint8_t* buf, uint64_t* cb_args)
{
    int fd_idx = fd - vm->io.fds;

    ssize_t len = write(fd->fd, buf, evt->len);
    cb_args[2] = len < 0 ? 0 : len;
    cb_args[1] = len < 0 ? errno : 0;

    make_func_procedure(evt->callback, evt->args, 0, vm);

    int rest_bufs = fd->n_write_bufs - fd_idx - 1;
    if (rest_bufs)
    {
        memmove(fd, fd + 1, rest_bufs * sizeof(io_mem_t));
    }

    fd->n_write_bufs--;
    if (fd->n_write_bufs &&
        fd->alloc_write_bufs - fd->n_write_bufs > IO_BUF_ALLOC)
    {
        fd->write_bufs = realloc(fd->write_bufs,
            sizeof(io_mem_t) * (fd->alloc_write_bufs - IO_BUF_ALLOC));
        fd->alloc_write_bufs -= IO_BUF_ALLOC;
    }
    vm->io.n_io_bufs--;
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
    uint32_t* args_ptr, vm_fd_t** fd)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 5 * 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return NULL;
    }

    if (!FITS_IN_32Bit(args[0]) || !FITS_IN_32Bit(args[1]) ||
        !FITS_IN_32Bit(args[2]) || !FITS_IN_32Bit(args[3]) ||
        !FITS_IN_32Bit(args[4]))
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return NULL;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(args[0]);
    if (fd_idx >= vm->io.n_fds || !vm->io.fds[fd_idx].used)
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

    *args_ptr = args[3];
    if (!deref(*args_ptr, 4 * 8, vm))
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return NULL;
    }

    *f_ptr = args[4];
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
    uint32_t args_ptr;
    vm_fd_t* fd;

    uint64_t* ret = get_io_task_params(vm, argv, retv,
        &ptr, &len, &f_ptr, &args_ptr, &fd);
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
    vm->io.n_io_bufs++;

    buf->ptr = ptr;
    buf->len = len;
    buf->callback = f_ptr;
    buf->args = args_ptr;

    *ret = 0;
}


void sys_write(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint32_t ptr;
    uint32_t len;
    uint32_t f_ptr;
    uint32_t args_ptr;
    vm_fd_t* fd;

    uint64_t* ret = get_io_task_params(vm, argv, retv,
        &ptr, &len, &f_ptr, &args_ptr, &fd);
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
    vm->io.n_io_bufs++;

    buf->ptr = ptr;
    buf->len = len;
    buf->callback = f_ptr;
    buf->args = args_ptr;

    *ret = 0;
}
