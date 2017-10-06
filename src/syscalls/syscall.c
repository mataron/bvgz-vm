#include <stdio.h>
#include "vm.h"
#include "syscall.h"


int nSyscalls = 0;


void setup_system_call_table()
{
    nSyscalls = 0;
    for (syscall_f* f = SyscallTable; *f; f++)
    {
        nSyscalls++;
    }
}


uint8_t* deref(uint32_t ref, uint32_t size, vm_t* vm)
{
    if (ref + size > vm->memsz)
    {
        return NULL;
    }
    return vm->memory + ref;
}


int ensure_nul_term_str(uint8_t* base, vm_t* vm)
{
    uint8_t* mem_end = vm->memory + vm->memsz;
    for (uint8_t* p = base; p < mem_end; p++)
    {
        if (!*p) return 0;
    }
    vm->exceptions |= VM_E_MemFault;
    return -1;
}


#define SYSCALL(name) \
extern void sys_ ## name \
    (struct _vm_t* vm, uint32_t argv, uint32_t retv);
#include "syscall.inc"
#undef SYSCALL

syscall_f SyscallTable[] = {
#define SYSCALL(name) sys_ ## name,
#include "syscall.inc"
#undef SYSCALL
    NULL
};
