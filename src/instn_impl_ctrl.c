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
    uint32_t iptr = arg_value(instn, 0);
    uint32_t argv = arg_value(instn, 1);
    uint32_t retv = arg_value(instn, 2);

    proc_t* this_proc = vm->procedures->data;
    push_call_stack(this_proc, retv, argv, this_proc->iptr, vm);

    this_proc->iptr = iptr;
    return 0;
}


int op_syscall_3(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_ret_0(instn_t* instn, vm_t* vm)
{
    proc_t* this_proc = vm->procedures->data;
    if (!this_proc->cstack_sz)
    {
        delete_current_procedure(vm);
        return 0;
    }

    uint32_t ret_address = pop_call_stack(this_proc, vm);
    if (ret_address == (uint32_t)-1)
    {
        delete_current_procedure(vm);
        return 0;
    }

    this_proc->iptr = ret_address;
    return 0;
}


int op_argv_1(instn_t* instn, vm_t* vm)
{
    uint32_t args = 0;

    proc_t* this_proc = vm->procedures->data;
    if (this_proc->cstack_sz)
    {
        uint32_t idx = this_proc->cstack_sz - 1;
        args = this_proc->cstack[idx].args;
    }

    lref32(instn->args[0].ptr) = args;
    return 0;
}


int op_retv_1(instn_t* instn, vm_t* vm)
{
    uint32_t retv = 0;

    proc_t* this_proc = vm->procedures->data;
    if (this_proc->cstack_sz)
    {
        uint32_t idx = this_proc->cstack_sz - 1;
        retv = this_proc->cstack[idx].retval;
    }
    else
    {
        vm->exceptions |= VM_E_MemFault;
        return -1;
    }

    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!ret)
    {
        return -1;
    }

    *ret = arg_value(instn, 0);
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
