#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
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
    uint8_t* args = NULL;
    uint64_t* ret = NULL;
    char* path = NULL;
    if (setup_sys_fs_call(vm, argv, retv, 5 * 4 + 8,
                          &args, &ret, &path) < 0)
    {
        return;
    }

    uint32_t buf_ref = *(uint32_t*)(args + 4);
    uint64_t buf_len = *(uint64_t*)(args + 8);
    uint32_t n_saved_entries_ref = *(uint32_t*)(args + 16);
    uint32_t n_total_entries_ref = *(uint32_t*)(args + 20);
    uint32_t min_buf_sz_ref = *(uint32_t*)(args + 24);

    uint8_t* buf = NULL;
    if (buf_len > 0)
    {
        buf = deref(buf_ref, buf_len, vm);
        if (!buf)
        {
            vm->error_no = EFAULT;
            *ret = 1;
            return;
        }
    }

    uint64_t* n_saved_entries = (uint64_t*)
        deref(n_saved_entries_ref, 4, vm);
    uint64_t* n_total_entries = (uint64_t*)
        deref(n_total_entries_ref, 4, vm);
    uint64_t* min_buf_sz = (uint64_t*)
        deref(min_buf_sz_ref, 4, vm);

    if (!buf && !min_buf_sz)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return;
    }

    DIR* dir = opendir(path);
    if (!dir)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    uint32_t saved_entries = 0;
    uint32_t total_entries = 0;
    uint32_t buf_used = 0;
    uint32_t buf_required = 4;
    uint8_t* end_of_buf = buf + buf_len;
    struct dirent *dent;

    *ret = 0;
    vm->error_no = errno = 0;
    while ((dent = readdir(dir)) != NULL)
    {
        if (errno)
        {
            vm->error_no = errno;
            *ret = 1;
            break;
        }

        int len = strlen(dent->d_name);
        buf_required += 5 + len;

        if (buf_used + len + 5 <= buf_len)
        {
            end_of_buf -= len + 1;
            assert(end_of_buf > buf + saved_entries * 4);

            memcpy(end_of_buf, dent->d_name, len + 1);

            *(uint32_t*)(buf + saved_entries * 4) =
                buf_ref + (uint32_t)(end_of_buf - buf);

            saved_entries++;
            *(uint32_t*)(buf + saved_entries * 4) = 0;
            buf_used += len + 5;
        }

        total_entries++;
    }

    if (n_saved_entries) *n_saved_entries = saved_entries;
    if (n_total_entries) *n_total_entries = total_entries;
    if (min_buf_sz) *min_buf_sz = buf_required;

    // DO NOT set the 'ret' past this point

    closedir(dir);
}
