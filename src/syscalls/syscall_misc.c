#include <string.h>
#include <time.h>
#include <errno.h>

#include "vm.h"
#include "bytecode.h"
#include "syscall.h"


void sys_getlasterr(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!ret)
    {
        vm->error_no = EFAULT;
        return;
    }

    *ret = vm->error_no;
    vm->error_no = 0;
}


void sys_setlasterr(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref(argv, 8, vm);
    if (!args)
    {
        vm->error_no = EFAULT;
        return;
    }

    vm->error_no = *args;
}


void sys_time(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref(argv, 2 * 8, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    struct timespec tp;
    if (clock_gettime(CLOCK_REALTIME, &tp) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *args = tp.tv_sec;
    *(args + 1) = tp.tv_nsec;

    *ret = 0;
}


void sys_timeout(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref(argv, 4 + 8, vm);
    uint64_t* ret = (uint64_t*)deref(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint32_t f_ptr = *(uint32_t*)args;
    uint64_t timeout_millis = *(uint64_t*)(args + 4);

    if (f_ptr > vm->codesz)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return;
    }

    struct timespec tp;
    if (clock_gettime(CLOCK_REALTIME, &tp) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    tp.tv_sec += MILLISECONDS_TO_SECONDS(timeout_millis);
    tp.tv_nsec += MILLISECONDS_TO_NANOSECONDS(timeout_millis);

    make_vm_timer(vm, &tp, f_ptr);
}


void sys_yield(vm_t* vm, uint32_t argv, uint32_t retv)
{
    if (!vm->procedures->next)
    {
        return;
    }

    list_t* self = vm->procedures;
    vm->procedures = vm->procedures->next;
    list_unlink(self);

    list_t* tail = list_tail(vm->procedures);
    list_append(tail, self);
}


void sys_codesz(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!ret)
    {
        vm->error_no = EFAULT;
        return;
    }

    *ret = vm->codesz;
}


void sys_codecp(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    if (args[0] + args[1] > vm->codesz)
    {
        vm->error_no = EINVAL;
        return;
    }

    uint8_t* ptr = deref_mem_ptr(args[2], args[1], vm);
    if (!ptr)
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return;
    }

    memcpy(ptr, vm->code + args[0], args[1]);
    *ret = 0;
}
