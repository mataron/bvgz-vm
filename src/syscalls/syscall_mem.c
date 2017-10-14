#include <stdlib.h>
#include <errno.h>

#include "vm.h"
#include "syscall.h"
#include "bytecode.h"


void sys_mexpand(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint64_t sz = args[0];
    void* mem = realloc(vm->memory, vm->memsz + sz);
    if (!mem)
    {
        vm->error_no = ENOMEM;
        *ret = 1;
        return;
    }

    vm->memory = mem;
    vm->memsz += sz;

    ret = mem + retv; // in case mem != vm->memory (old)
    *ret = 0;
}


void sys_mretract(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint64_t sz = args[0];
    if (sz > vm->memsz)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return;
    }

    void* mem = realloc(vm->memory, vm->memsz - sz);
    if (!mem)
    {
        vm->error_no = ENOMEM;
        *ret = 1;
        return;
    }

    vm->memory = mem;
    vm->memsz -= sz;

    ret = mem + retv; // in case mem != vm->memory (old)
    *ret = 0;
}


void sys_msize(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!ret)
    {
        vm->error_no = EFAULT;
        return;
    }

    *ret = vm->memsz;
}
