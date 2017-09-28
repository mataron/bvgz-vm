#include <stdlib.h>
#include "vm.h"


proc_t* make_procedure(uint32_t iptr, vm_t* vm)
{
    if (iptr >= vm->codesz)
    {
        vm->exceptions |= VM_E_BadInstnPointer;
        return NULL;
    }

    proc_t* proc = malloc(sizeof(proc_t));
    proc->iptr = iptr;
    proc->cstack = NULL;
    proc->cstack_sz = 0;
    proc->cstack_alloc = 0;

    list_t* proc_node = list_make_node(proc);

    list_t* tail = list_tail(vm->procedures);
    list_append(tail, proc_node);

    if (!vm->procedures)
    {
        vm->procedures = proc_node;
    }

    return proc;
}


proc_t* make_func_procedure(uint32_t iptr, uint8_t* argv,
    uint8_t* retv, vm_t* vm)
{
    proc_t* proc = make_procedure(iptr, vm);
    if (proc)
    {
        push_call_stack(proc, retv, argv, (uint32_t)-1, vm);
    }
    return proc;
}


void push_call_stack(proc_t* proc, uint8_t* retval,
    uint8_t* args, uint32_t ret_address, vm_t* vm)
{
    if (proc->cstack_alloc <= proc->cstack_sz + 1)
    {
        proc->cstack_alloc += FUNC_STACK_ALLOC_SZ;
        proc->cstack = realloc(proc->cstack,
            sizeof(fcall_t) * proc->cstack_alloc);
    }

    proc->cstack[proc->cstack_sz].retval = retval;
    proc->cstack[proc->cstack_sz].args = args;
    proc->cstack[proc->cstack_sz].ret_address = ret_address;
    proc->cstack_sz++;
}
