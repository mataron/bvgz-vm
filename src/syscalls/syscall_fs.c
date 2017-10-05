#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "vm.h"
#include "syscall_io.h"
#include "bytecode.h"


// -- file management

#define FS_OPEN_FLAG_MASK   \
    O_RDONLY | O_RDWR | O_WRONLY | O_APPEND | O_CREAT


int sys_fs_open(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref_mem_ptr(argv, 4 + 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        return -1;
    }

    uint8_t* path = deref_mem_ptr(*(uint32_t*)args, 1, vm);
    if (path == NULL)
    {
        return -1;
    }
    if (ensure_nul_term_str(path, vm) < 0)
    {
        return -1;
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
        return 0;
    }

    return FD_IDX_TO_HANDLE(fd);
}


int sys_fs_stat(vm_t* vm, uint32_t argv, uint32_t retv)
{
    return 0;
}


int sys_fs_unlink(vm_t* vm, uint32_t argv, uint32_t retv)
{
    return 0;
}


int sys_fs_seek(vm_t* vm, uint32_t argv, uint32_t retv)
{
    return 0;
}


// -- directory management

int sys_fs_mkdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
    return 0;
}


int sys_fs_rmdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
    return 0;
}


int sys_fs_readdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
    return 0;
}
