#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "vm.h"
#include "syscall.h"


void sys_exec(vm_t* vm, uint32_t argv, uint32_t retv)
{
    
}


void sys_run(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_kill(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint64_t pid = args[0];

    int ret = kill(pid, SIGTERM);
    if (ret < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *ret = 0;
}


void sys_onexit(vm_t* vm, uint32_t argv, uint32_t retv)
{
}
