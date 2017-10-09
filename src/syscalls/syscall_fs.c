#include <sys/types.h>
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


static int setup_sys_fs_call(vm_t* vm, uint32_t argv, uint32_t retv,
    uint32_t argslen, uint8_t** args, uint64_t** ret, char** path)
{
    *args = deref(argv, argslen, vm);
    *ret = (uint64_t*)deref(retv, 8, vm);
    if (!*args || !*ret)
    {
        vm->error_no = EFAULT;
        if (*ret) **ret = 1;
        return -1;
    }

    *path = (char*)deref(*(uint32_t*)*args, 1, vm);
    if (*path == NULL)
    {
        vm->error_no = EFAULT;
        **ret = 1;
        return -1;
    }
    if (ensure_nul_term_str((uint8_t*)*path, vm) < 0)
    {
        vm->error_no = EINVAL;
        **ret = 1;
        return -1;
    }

    return 0;
}


void sys_fs_open(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = NULL;
    uint64_t* ret = NULL;
    char* path = NULL;
    if (setup_sys_fs_call(vm, argv, retv, 4 + 8,
                          &args, &ret, &path) < 0)
    {
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
    uint8_t* args = NULL;
    uint64_t* ret = NULL;
    char* path = NULL;
    if (setup_sys_fs_call(vm, argv, retv, 2 * 4,
                          &args, &ret, &path) < 0)
    {
        return;
    }

    uint32_t stat_ptr = *(uint32_t*)(args + 4);
    struct stat* stat_s = (struct stat*)
        deref_mem_ptr(stat_ptr, sizeof(struct stat), vm);
    if (stat_s == NULL)
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return;
    }

    *ret = 0;
    if (stat(path, stat_s) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
    }
}


void sys_fs_unlink(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = NULL;
    uint64_t* ret = NULL;
    char* path = NULL;
    if (setup_sys_fs_call(vm, argv, retv, 4,
                          &args, &ret, &path) < 0)
    {
        return;
    }

    if (unlink((char*)path) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *ret = 0;
}


void sys_fs_seek(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 3 * 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint32_t fd_idx = FD_HANDLE_TO_IDX(args[0]);
    if (fd_idx >= vm->io.n_fds || !vm->io.fds[fd_idx].used)
    {
        vm->error_no = EBADF;
        *ret = 1;
        return;
    }

    int64_t offset = *(int64_t*)(args + 1);
    uint64_t whence = args[2];
    if (whence > 2)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return;
    }

    off_t result = lseek(vm->io.fds[fd_idx].fd, offset, whence);
    if (result == (off_t)-1)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *ret = 0;
}


// -- directory management

void sys_fs_mkdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = NULL;
    uint64_t* ret = NULL;
    char* path = NULL;
    if (setup_sys_fs_call(vm, argv, retv, 4,
                          &args, &ret, &path) < 0)
    {
        return;
    }

    mode_t mask = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    if (mkdir((char*)path, mask) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *ret = 0;
}


void sys_fs_rmdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = NULL;
    uint64_t* ret = NULL;
    char* path = NULL;
    if (setup_sys_fs_call(vm, argv, retv, 4,
                          &args, &ret, &path) < 0)
    {
        return;
    }

    if (rmdir((char*)path) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *ret = 0;
}


void sys_fs_readdir(vm_t* vm, uint32_t argv, uint32_t retv)
{
}
