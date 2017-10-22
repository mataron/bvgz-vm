#include "debug_util.h"
#include "instns/instn.h"
#include "bytecode.h"


static dbg_break_pt_t* has_hit_breakpoint(dbg_state_t* state, instn_t* instn)
{
    for (list_t* n = state->breakpoints; n; n = n->next)
    {
        dbg_break_pt_t* brk = n->data;
        switch (brk->type)
        {
        case BRK_T_Address:
            if (brk->point.address == state->vm->iptr) return brk;
            continue;
        case BRK_T_Label:
            if (brk->point.label.address == state->vm->iptr) return brk;
            continue;
        case BRK_T_Line:
            if (brk->point.line.address == state->vm->iptr) return brk;
            continue;
        case BRK_T_InstnName:
            if (brk->point.instn_name == InstnDefs[instn->code >> 3].name)
                return brk;
            continue;
        case BRK_T_InstnCount:
            if (brk->point.instn_count == state->vm->instns) return brk;
            continue;
        }
    }
    return NULL;
}


static int step_program(dbg_state_t* state, instn_t* instn,
    int check_breakpoints)
{
    while (1)
    {
        if (!state->vm->procedures)
        {
            int n_events = vm_fire_events(state->vm);
            if (n_events < 0)
            {
                break;
            }
            if (n_events == 0)
            {
                continue;
            }
        }

        proc_t* proc = vm_current_procedure(state->vm);
        if (!proc)
        {
            continue;
        }

        if (vm_decode_instn(state->vm, proc, instn) < 0)
        {
            break;
        }

        if (check_breakpoints)
        {
            dbg_break_pt_t* brk = has_hit_breakpoint(state, instn);
            if (brk)
            {
                printf("Reached breakpoint ");
                print_breakpoint(brk, state);
                printf("\n");
                return 2;
            }
        }

        if (vm_exec_instn(state->vm, instn) < 0)
        {
            break;
        }

        state->vm->instns++;
        state->vm->instns_since_last_cleanup++;

        if (0 ==
            state->vm->instns_since_last_cleanup % VM_CLEANUP_PERIOD_INSTNS)
        {
            cleanup_vm(state->vm);
        }

        return 1;
    }

    return 0;
}


int dbg_run(int argc, char** argv, dbg_state_t* state)
{
    instn_t instn;
    state->flags = F_RUNNING;

    while (1)
    {
        int ret = step_program(state, &instn, 1);
        if (ret == 0) break;
        if (ret == 2) {
            print_code_address(state->vm->iptr, state);
            printf(":  ");
            print_instn(&instn, state);
            printf("\n");
            return 0;
        }
    }

    state->flags &= ~F_RUNNING;
    printf("program exited\n");
    return 0;
}


int dbg_step(int argc, char** argv, dbg_state_t* state)
{
    if (!(state->flags & F_RUNNING))
    {
        printf("program is not running!\n");
        return 0;
    }

    instn_t instn;
    int ret = step_program(state, &instn, 0);
    if (!ret)
    {
        printf("exec instn failed\n");
    }

    print_code_address(state->vm->iptr, state);
    printf(":  ");
    print_instn(&instn, state);
    printf("\n");

    return 0;
}


int dbg_cleanup(int argc, char** argv, dbg_state_t* state)
{
    if (!(state->flags & F_RUNNING))
    {
        printf("program is not running!\n");
        return 0;
    }

    cleanup_vm(state->vm);
    return 0;
}


int dbg_fire_events(int argc, char** argv, dbg_state_t* state)
{
    if (!(state->flags & F_RUNNING))
    {
        printf("program is not running!\n");
        return 0;
    }

    if (!has_pending_events(state->vm))
    {
        printf("no pending events\n");
        return 0;
    }

    int n_events = vm_fire_events(state->vm);
    if (n_events < 0)
    {
        printf("events firing failed\n");
        return 0;
    }

    printf("events fired: %d\n", n_events);
    return 0;
}
