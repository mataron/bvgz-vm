#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

#include "config.h"
#include "vm.h"
#include "syscall_io.h"
#include "bytecode.h"


#define FD_ALLOC_STEP   5


void destroy_vm_io(vm_t* vm)
{
    if (!vm || !vm->io.fds) return;

    for (uint32_t n = 0; n < vm->io.n_fds; n++)
    {
        vm_fd_t* fd = vm->io.fds + n;
        if (!fd->used) continue;

        free(fd->events);
        close(fd->fd);
    }

    free(vm->io.fds);
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
            vm->io.fds[i].on_close = NULL;
            vm->io.used_fds++;
            return i;
        }
    }

    // unreachable!!!
    abort();
    return (uint64_t)-1;
}


void dealloc_fd(uint64_t fd_idx, vm_t* vm)
{
    vm_fd_t* fd = vm->io.fds + fd_idx;

    if (fd->on_close) fd->on_close(vm, fd);

    fd->used = 0;
    free(fd->events);
    vm->io.used_fds--;
}


static uint32_t read_io_evt_handler(vm_t* vm, vm_fd_t* fd,
    vm_io_evt_t* evt, int* remove);
static uint32_t write_io_evt_handler(vm_t* vm, vm_fd_t* fd,
    vm_io_evt_t* evt, int* remove);


static int setup_select_rwsets(int maxfd, vm_fd_t* fd,
    fd_set* readset, fd_set* writeset)
{
    int set_r = 0, set_w = 0;
    for (uint32_t e = 0;
         e < fd->n_events && (!set_r || !set_w); e++)
    {
        vm_io_evt_t* evt = fd->events + e;
        set_r |= evt->flags & IO_EVT_SELECT_READ;
        set_w |= evt->flags & IO_EVT_SELECT_WRITE;
    }
    if (set_r || set_w)
    {
        if (set_r) FD_SET(fd->fd, readset);
        if (set_w) FD_SET(fd->fd, writeset);
        if (maxfd < fd->fd) maxfd = fd->fd;
    }
    return maxfd;
}


static uint32_t activate_event(vm_t* vm, vm_fd_t* fd, int flag_mask)
{
    for (uint32_t e = 0; e < fd->n_events; e++)
    {
        vm_io_evt_t* evt = fd->events + e;
        if (evt->flags != flag_mask) continue;

        int remove = 1;
        uint32_t result = evt->activate(vm, fd, evt, &remove);

        if (remove)
        {
            if (e != fd->n_events - 1)
            {
                fd->events[e] = fd->events[fd->n_events - 1];
            }

            fd->n_events--;
            if (fd->n_events &&
                fd->alloc_events - fd->n_events > IO_EVT_ALLOC)
            {
                fd->events = realloc(fd->events,
                    sizeof(io_mem_t) * (fd->alloc_events - IO_EVT_ALLOC));
                fd->alloc_events -= IO_EVT_ALLOC;
            }
            vm->io.n_io_events--;

            e--;
        }

        return result;
    }

    return 0;
}


uint32_t fire_io_events(vm_t* vm)
{
    struct timespec tm = { 0, 0 };
    fd_set readset, writeset;
    int maxfd = 0;

    if (!vm->io.n_io_events)
    {
        return 0;
    }

    FD_ZERO(&readset);
    FD_ZERO(&writeset);

    for (uint32_t fd_idx = 0; fd_idx < vm->io.n_fds; fd_idx++)
    {
        vm_fd_t* fd = vm->io.fds + fd_idx;
        if (!fd->used) continue;
        if (fd->n_events)
        {
            maxfd = setup_select_rwsets(maxfd, fd, &readset, &writeset);
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
        if (FD_ISSET(fd->fd, &readset))
        {
            events += activate_event(vm, fd, IO_EVT_SELECT_READ);
        }
        if (FD_ISSET(fd->fd, &writeset))
        {
            events += activate_event(vm, fd, IO_EVT_SELECT_WRITE);
        }
    }

    return events;
}


int has_pending_io_events(vm_t* vm)
{
    return vm->io.n_io_events > 0;
}


static int common_io_evt_handler(vm_t* vm, vm_fd_t* fd,
    vm_io_evt_t* evt, uint8_t** buf, uint64_t** cb_args)
{
    io_mem_t* io_evt = (io_mem_t*)evt->data;
    *cb_args = (uint64_t*)deref(io_evt->callback.args, 4 * 8, vm);
    if (!*cb_args)
    {
        // not much to do...
        vm->error_no = EFAULT;
        return -1;
    }

    (*cb_args)[0] = FD_IDX_TO_HANDLE(fd - vm->io.fds);
    (*cb_args)[1] = 0; // errno
    (*cb_args)[2] = 0; // bytes read

    *buf = deref(io_evt->ptr, io_evt->len, vm);
    if (!*buf)
    {
        (*cb_args)[1] = EFAULT;
        return -1;
    }

    return 0;
}

static uint32_t read_io_evt_handler(vm_t* vm, vm_fd_t* fd,
    vm_io_evt_t* evt, int* remove)
{
    *remove = 1;

    uint8_t* buf = NULL;
    uint64_t* cb_args = NULL;
    if (common_io_evt_handler(vm, fd, evt, &buf, &cb_args) < 0)
    {
        return 0;
    }

    io_mem_t* io_evt = (io_mem_t*)evt->data;

    ssize_t len = read(fd->fd, buf, io_evt->len);
    cb_args[2] = len < 0 ? 0 : len;
    cb_args[1] = len < 0 ? errno : 0;
    if (len < 0)
    {
        vm->error_no = errno;
    }

    make_func_procedure(io_evt->callback.callback,
        io_evt->callback.args, 0, vm);

    free(evt->data);
    evt->data = NULL;
    return 1;
}


static uint32_t write_io_evt_handler(vm_t* vm, vm_fd_t* fd,
    vm_io_evt_t* evt, int* remove)
{
    *remove = 1;

    uint8_t* buf = NULL;
    uint64_t* cb_args = NULL;
    if (common_io_evt_handler(vm, fd, evt, &buf, &cb_args) < 0)
    {
        return 0;
    }

    io_mem_t* io_evt = (io_mem_t*)evt->data;

    ssize_t len = write(fd->fd, buf, io_evt->len);
    cb_args[2] = len < 0 ? 0 : len;
    cb_args[1] = len < 0 ? errno : 0;
    if (len < 0)
    {
        vm->error_no = errno;
    }

    make_func_procedure(io_evt->callback.callback,
        io_evt->callback.args, 0, vm);

    free(evt->data);
    evt->data = NULL;
    return 1;
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
    uint8_t* args = deref_mem_ptr(argv, 5 * 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return NULL;
    }

    uint64_t fd_arg = *(uint64_t*)args;
    uint32_t fd_idx = FD_HANDLE_TO_IDX(fd_arg);
    if (fd_idx >= vm->io.n_fds || !vm->io.fds[fd_idx].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return NULL;
    }
    *fd = vm->io.fds + fd_idx;

    *ptr = *(uint32_t*)(args + 8);
    *len = *(uint64_t*)(args + 12);

    if (!deref(*ptr, *len, vm))
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return NULL;
    }

    *args_ptr = *(uint32_t*)(args + 20);
    if (!deref(*args_ptr, 4 * 8, vm))
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return NULL;
    }

    *f_ptr = *(uint32_t*)(args + 24);
    if (*f_ptr >= vm->codesz)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return NULL;
    }

    return ret;
}


vm_io_evt_t* alloc_event(vm_t* vm, vm_fd_t* fd)
{
    if (fd->n_events + 1 > fd->alloc_events)
    {
        fd->events = realloc(fd->events,
            sizeof(io_mem_t) * (fd->alloc_events + IO_EVT_ALLOC));
        fd->alloc_events += IO_EVT_ALLOC;
    }

    vm_io_evt_t* evt = fd->events + fd->n_events;
    evt->data = NULL;

    fd->n_events++;
    vm->io.n_io_events++;

    return evt;
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

    vm_io_evt_t* evt = alloc_event(vm, fd);

    evt->activate = read_io_evt_handler;
    evt->flags = IO_EVT_SELECT_READ;
    evt->data = malloc(sizeof(io_mem_t));

    io_mem_t* buf = (io_mem_t*)evt->data;

    buf->ptr = ptr;
    buf->len = len;
    buf->callback.callback = f_ptr;
    buf->callback.args = args_ptr;

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

    vm_io_evt_t* evt = alloc_event(vm, fd);

    evt->activate = write_io_evt_handler;
    evt->flags = IO_EVT_SELECT_WRITE;
    evt->data = malloc(sizeof(io_mem_t));

    io_mem_t* buf = (io_mem_t*)evt->data;

    buf->ptr = ptr;
    buf->len = len;
    buf->callback.callback = f_ptr;
    buf->callback.args = args_ptr;

    *ret = 0;
}
