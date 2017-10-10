#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "vm.h"
#include "syscall_net.h"


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

    vm->io.fds[fd].fd = r;

    *ret = FD_IDX_TO_HANDLE(fd);
}


void sys_bind(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_connect(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_listen(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_accept(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_peer_address(vm_t* vm, uint32_t argv, uint32_t retv)
{
}
