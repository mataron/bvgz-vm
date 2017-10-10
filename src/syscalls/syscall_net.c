#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "vm.h"
#include "syscall_net.h"


void net_io_close_handler(vm_t* vm, vm_fd_t* fd)
{
    free(fd->data);
}


void sys_socket(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args = (uint64_t*)deref(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint64_t fd = alloc_fd(vm);

    int r = -1;
    errno = 0;
    switch (args[0])
    {
    case SOCK_T_TCP:
        r = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        break;
    case SOCK_T_UDP:
        r = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        break;
    }

    if (r < 0)
    {
        dealloc_fd(fd, vm);
        vm->error_no = errno ? errno : EINVAL;
        *ret = 1;
        return;
    }

    fcntl(r, F_SETFL, O_NONBLOCK);
    vm->io.fds[fd].fd = r;
    vm->io.fds[fd].on_close = net_io_close_handler;

    *ret = FD_IDX_TO_HANDLE(fd);
}


void sys_bind(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref(argv, 8 + 4, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(*(uint64_t*)args);
    if (fd_idx >= vm->io.n_fds || !vm->io.fds[fd_idx].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return;
    }

    uint32_t addr_ref = *(uint32_t*)(args + 8);
    uint8_t* addr_ptr = deref(addr_ref, 6, vm);
    if (!addr_ptr)
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = *(uint32_t*)addr_ptr;
    addr.sin_port = *(uint16_t*)(addr_ptr + 4);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    int r = bind(vm->io.fds[fd_idx].fd,
        (struct sockaddr*)&addr, sizeof(addr));
    if (r < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *ret = 0;
}


static uint32_t connect_io_evt_handler(vm_t* vm, vm_fd_t* fd,
    vm_io_evt_t* evt, int* remove)
{
    *remove = 1;

    vm_net_cb_t* io_evt = (vm_net_cb_t*)evt->data;
    uint64_t* cb_args = (uint64_t*)deref(io_evt->args, 2 * 8, vm);
    if (!cb_args)
    {
        // not much to do...
        vm->error_no = EFAULT;
        return 0;
    }

    cb_args[0] = FD_IDX_TO_HANDLE(fd - vm->io.fds);

    vm_net_sock_t* net = (vm_net_sock_t*)fd->data;
    int r = connect(fd->fd, (struct sockaddr*)&net->addr,
        sizeof(net->addr));
    if ((r < 0 && errno != EINPROGRESS) || r == 0)
    {
        vm_net_cb_t* conn = (vm_net_cb_t*)evt->data;

        cb_args[1] = r == 0 ? 0 : errno;
        make_func_procedure(conn->callback, conn->args, 0, vm);
        
        free(evt->data);
        evt->data = NULL;
        return 1;
    }

    // connect() is still in progress:
    *remove = 0;
    return 0;
}


void sys_connect(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref(argv, 8 + 3 * 4, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(*(uint64_t*)args);
    if (fd_idx >= vm->io.n_fds || !vm->io.fds[fd_idx].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return;
    }
    vm_fd_t* fd = vm->io.fds + fd_idx;

    uint32_t addr_ref = *(uint32_t*)(args + 8);
    uint8_t* addr_ptr = deref(addr_ref, 6, vm);
    if (!addr_ptr)
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return;
    }

    fd->data = malloc(sizeof(vm_net_sock_t));
    vm_net_sock_t* data = fd->data;

    data->addr.sin_family = AF_INET;
    data->addr.sin_addr.s_addr = *(uint32_t*)addr_ptr;
    data->addr.sin_port = *(uint16_t*)(addr_ptr + 4);
    memset(data->addr.sin_zero, 0, sizeof(data->addr.sin_zero));

    vm_io_evt_t* evt = alloc_event(vm, fd);
    
    evt->activate = connect_io_evt_handler;
    evt->flags = IO_EVT_SELECT_WRITE;
    evt->data = malloc(sizeof(vm_net_cb_t));

    vm_net_cb_t* buf = (vm_net_cb_t*)evt->data;

    buf->args = *(uint32_t*)(args + 12);
    buf->callback = *(uint32_t*)(args + 16);

    *ret = 0;
}


void sys_listen(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args = (uint64_t*)deref(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(*args);
    if (fd_idx >= vm->io.n_fds || !vm->io.fds[fd_idx].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return;
    }

    int fd = vm->io.fds[fd_idx].fd;
    int param = 1;
    int r = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
        &param, sizeof(param));
    if (r < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    r = listen(fd, LISTEN_BACKLOG);
    if (r < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *ret = 0;
}


static uint32_t accept_io_evt_handler(vm_t* vm, vm_fd_t* fd,
    vm_io_evt_t* evt, int* remove)
{
    // continue on accept()ing clients!
    *remove = 0;

    vm_net_cb_t* io_evt = (vm_net_cb_t*)evt->data;
    uint64_t* cb_args = (uint64_t*)deref(io_evt->args, 2 * 8, vm);
    if (!cb_args)
    {
        // not much to do...
        vm->error_no = EFAULT;
        *remove = 1;
        return 0;
    }

    struct sockaddr_in peer_addr;
    socklen_t len = sizeof(peer_addr);
    int r = accept(fd->fd, (struct sockaddr*)&peer_addr, &len);
    if ((r < 0 && errno != EAGAIN) || r >= 0)
    {
        vm_net_cb_t* cb = (vm_net_cb_t*)evt->data;
        if (r >= 0)
        {
            uint64_t newfd_idx = alloc_fd(vm);
            vm_fd_t* newfd = vm->io.fds + newfd_idx;
            fcntl(r, F_SETFL, O_NONBLOCK);

            newfd->fd = r;
            newfd->on_close = net_io_close_handler;

            newfd->data = malloc(sizeof(vm_net_sock_t));
            vm_net_sock_t* data = newfd->data;
            memcpy(&data->addr, &peer_addr, sizeof(peer_addr));

            cb_args[0] = FD_IDX_TO_HANDLE(newfd_idx);
        }

        cb_args[1] = r == 0 ? 0 : errno;
        make_func_procedure(cb->callback, cb->args, 0, vm);

        return 1;
    }

    // no new clients
    return 0;
}


void sys_accept(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref(argv, 8 + 2 * 4, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(*(uint64_t*)args);
    if (fd_idx >= vm->io.n_fds || !vm->io.fds[fd_idx].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return;
    }
    vm_fd_t* fd = vm->io.fds + fd_idx;

    vm_io_evt_t* evt = alloc_event(vm, fd);
    
    evt->activate = accept_io_evt_handler;
    evt->flags = IO_EVT_SELECT_READ;
    evt->data = malloc(sizeof(vm_net_cb_t));

    vm_net_cb_t* buf = (vm_net_cb_t*)evt->data;

    buf->args = *(uint32_t*)(args + 8);
    buf->callback = *(uint32_t*)(args + 12);

    *ret = 0;
}


void sys_peer_address(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref(argv, 8 + 4, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(*(uint64_t*)args);
    vm_fd_t* fd = vm->io.fds + fd_idx;
    if (fd_idx >= vm->io.n_fds || !fd->used || !fd->data)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return;
    }

    uint32_t addr_ref = *(uint32_t*)(args + 8);
    uint8_t* addr_ptr = deref(addr_ref, 6, vm);
    if (!addr_ptr)
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return;
    }

    vm_net_sock_t* data = (vm_net_sock_t*)fd->data;

    *(uint32_t*)addr_ptr = data->addr.sin_addr.s_addr;
    *(uint16_t*)(addr_ptr + 4) = data->addr.sin_port;

    *ret = 0;
}
