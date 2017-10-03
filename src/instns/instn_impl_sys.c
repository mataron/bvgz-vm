#include "vm.h"
#include "instn.h"
#include "instn_impl.h"
#include "syscalls/syscall.h"


int op_syscall_3(instn_t* instn, vm_t* vm)
{
    fire_vm_events(vm);

    uint16_t syscall_id = arg_value(instn, 0);
    uint32_t argv = arg_value(instn, 1);
    uint32_t retv = arg_value(instn, 2);

    if (syscall_id >= nSyscalls)
    {
        vm->exceptions |= VM_E_BadSyscallNo;
        return -1;
    }

    SyscallTable[syscall_id](vm, argv, retv);
    return 0;
}
