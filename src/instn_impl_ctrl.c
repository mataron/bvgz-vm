#include "vm.h"
#include "instn.h"
#include "instn_impl.h"


static int do_jump(uint32_t iptr, vm_t* vm)
{
    if (iptr >= vm->codesz)
    {
        vm->exceptions |= VM_E_BadInstnPointer;
        return -1;
    }

    proc_t* this_proc = vm->procedures->data;
    this_proc->iptr = iptr;
    return 0;
}


int op_jmp_1(instn_t* instn, vm_t* vm)
{
    return do_jump(arg_value(instn, 0), vm);
}


int op_jtrue_2(instn_t* instn, vm_t* vm)
{
    if (arg_value(instn, 1))
    {
        return do_jump(arg_value(instn, 0), vm);
    }
    return 0;
}


int op_jfalse_2(instn_t* instn, vm_t* vm)
{
    if (!arg_value(instn, 1))
    {
        return do_jump(arg_value(instn, 0), vm);
    }
    return 0;
}


int op_call_3(instn_t* instn, vm_t* vm)
{
    uint8_t* argv = instn->args[1].ptr;
    uint8_t* retv = instn->args[2].ptr;
    uint32_t iptr = arg_value(instn, 0);

    if (!make_func_procedure(iptr, argv, retv, vm))
    {
        return -1;
    }

    return 0;
}


int op_syscall_3(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_ret_0(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_argv_1(instn_t* instn, vm_t* vm)
{
    uint64_t args = 0;

    proc_t* this_proc = vm->procedures->data;
    if (this_proc->cstack_sz)
    {
        uint32_t idx = this_proc->cstack_sz - 1;
        args = (uint64_t) this_proc->cstack[idx].args;
    }

    lref64(instn->args[0].ptr) = args;
    return 0;
}


int op_retv_1(instn_t* instn, vm_t* vm)
{
    uint64_t retv = 0;

    proc_t* this_proc = vm->procedures->data;
    if (this_proc->cstack_sz)
    {
        uint32_t idx = this_proc->cstack_sz - 1;
        retv = (uint64_t) this_proc->cstack[idx].retval;
    }

    lref64(instn->args[0].ptr) = retv;
    return 0;
}


int op_yield_0(instn_t* instn, vm_t* vm)
{
    list_t* next = vm->procedures->next;
    if (!next)
    {
        return 0;
    }

    list_t* current = vm->procedures;
    list_unlink(current);
    vm->procedures = next;

    list_t* tail = list_tail(vm->procedures);
    list_append(tail, current);

    return 0;
}
