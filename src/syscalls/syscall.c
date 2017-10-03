#include <stdio.h>
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


#define SYSCALL(name) \
extern int sys_ ## name \
    (struct _vm_t* vm, uint32_t argv, uint32_t retv);
#include "syscall.inc"
#undef SYSCALL

syscall_f SyscallTable[] = {
#define SYSCALL(name) sys_ ## name,
#include "syscall.inc"
#undef SYSCALL
    NULL
};
