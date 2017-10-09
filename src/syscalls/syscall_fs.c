#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "vm.h"
#include "syscall_io.h"
#include "bytecode.h"


// -- file management

#define FS_OPEN_FLAG_MASK   \
    O_RDONLY | O_RDWR | O_WRONLY | O_APPEND | O_CREAT


void sys_fs_open(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref(argv, 4 + 8, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint8_t* path = deref(*(uint32_t*)args, 1, vm);
    if (path == NULL)
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return;
    }
    if (ensure_nul_term_str(path, vm) < 0)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return;
    }

    uint64_t options = *(uint64_t*)(args + 4);
    options &= FS_OPEN_FLAG_MASK;
    options |= O_SYNC;

    uint64_t fd = alloc_fd(vm);

    vm->io.fds[fd].fd = open((char*)path, options);
    if (vm->io.fds[fd].fd < 0)
    {
        dealloc_fd(fd, vm);
        vm->error_no = errno;
        *ret = 0;
        return;
    }

    *ret = FD_IDX_TO_HANDLE(fd);
}


void sys_fs_stat(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_fs_unlink(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref(argv, 4, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint8_t* path = deref(*(uint32_t*)args, 1, vm);
    if (path == NULL)
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return;
    }
    if (ensure_nul_term_str(path, vm) < 0)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return;
    }

    *ret = 0;
    if (unlink((char*)path) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
    }
}


void sys_fs_seek(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


// -- directory management

void sys_fs_mkdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_fs_rmdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_fs_readdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
}
