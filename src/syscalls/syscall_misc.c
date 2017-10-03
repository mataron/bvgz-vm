#include <time.h>
#include <errno.h>

#include "vm.h"
#include "bytecode.h"
#include "syscall.h"


int sys_getlasterr(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!ret)
    {
        return -1;
    }

    *ret = vm->error_no;
    vm->error_no = 0;
    return 0;
}


int sys_setlasterr(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 8, vm);
    if (!args)
    {
        return -1;
    }

    vm->error_no = *args;
    return 0;
}


int sys_time(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args =  (uint64_t*)deref_mem_ptr(argv, 2 * 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        return -1;
    }

    struct timespec tp;
    if (clock_gettime(CLOCK_REALTIME, &tp) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return -1;
    }

    *args = tp.tv_sec;
    *(args + 1) = tp.tv_nsec;

    *ret = 0;
    return 0;
}


int sys_timeout(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref_mem_ptr(argv, 4 + 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        return -1;
    }

    uint32_t f_ptr = *(uint32_t*)args;
    uint64_t timeout_millis = *(uint64_t*)(args + 4);

    if (f_ptr > vm->codesz)
    {
        vm->exceptions |= VM_E_BadInstnPointer;
        return -1;
    }

    struct timespec tp;
    if (clock_gettime(CLOCK_REALTIME, &tp) < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return -1;
    }

    tp.tv_sec += MILLISECONDS_TO_SECONDS(timeout_millis);
    tp.tv_nsec += MILLISECONDS_TO_NANOSECONDS(timeout_millis);

    make_vm_timer(vm, &tp, f_ptr);
    return 0;
}


int sys_yield(vm_t* vm, uint32_t argv, uint32_t retv)
{
    if (!vm->procedures->next)
    {
        return 0;
    }

    list_t* self = vm->procedures;
    vm->procedures = vm->procedures->next;
    list_unlink(self);

    list_t* tail = list_tail(vm->procedures);
    list_append(tail, self);
    return 0;
}
