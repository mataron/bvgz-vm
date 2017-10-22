#include "debug_util.h"


int dbg_procs(int argc, char** argv, dbg_state_t* state)
{
    if (!(state->flags & F_RUNNING))
    {
        printf("program is not running!\n");
        return 0;
    }

    printf("procedures: %u\n", list_size(state->vm->procedures));

    uint32_t index = 0;
    for (list_t* p = state->vm->procedures; p; p = p->next, index++)
    {
        proc_t* proc = p->data;
        printf(" proc[%u]\n", index);
        printf("   i-ptr: ");
        print_code_address(proc->iptr, state, 0);
        printf("\n   stack size: %u\n", proc->cstack_sz);
    }

    return 0;
}


int dbg_stack(int argc, char** argv, dbg_state_t* state)
{
    if (!(state->flags & F_RUNNING))
    {
        printf("program is not running!\n");
        return 0;
    }

    if (!state->vm->procedures)
    {
        printf("no current procedure\n");
        return 0;
    }

    proc_t* proc = state->vm->procedures->data;
    if (!proc->cstack_sz)
    {
        printf("stack is empty\n");
        return 0;
    }

    for (uint32_t s = 0; s < proc->cstack_sz; s++)
    {
        fcall_t* f = proc->cstack + s;
        printf("[%2u]: ret=", s);
        print_code_address(f->ret_address, state, 0);
        printf("\targv=");
        print_mem_address(f->args, state);
        printf("\tretv=");
        print_mem_address(f->retval, state);
        printf("\n");
    }

    return 0;
}


void sys_yield(vm_t* vm, uint32_t argv, uint32_t retv);

int dbg_yield(int argc, char** argv, dbg_state_t* state)
{
    if (!(state->flags & F_RUNNING))
    {
        printf("program is not running!\n");
        return 0;
    }

    sys_yield(state->vm, 0, 0);
    return 0;
}
