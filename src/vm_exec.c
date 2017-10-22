#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "config.h"
#include "vm.h"
#include "instns/instn.h"
#include "bytecode.h"


static struct timespec idle_step = {
    MILLISECONDS_TO_SECONDS(IDLE_STEP_DURATION_MS),
    MILLISECONDS_TO_NANOSECONDS(IDLE_STEP_DURATION_MS)
};


void execute_vm(vm_t* vm)
{
    instn_t instn;

    while (1)
    {
        if (!vm->procedures)
        {
            int n_events = vm_fire_events(vm);
            if (n_events < 0)
            {
                break;
            }
            if (n_events == 0)
            {
                continue;
            }
        }

        proc_t* proc = vm_current_procedure(vm);
        if (!proc)
        {
            continue;
        }

        if (vm_decode_instn(vm, proc, &instn) < 0)
        {
            break;
        }

        if (vm_exec_instn(vm, &instn) < 0)
        {
            break;
        }

        vm->instns++;
        vm->instns_since_last_cleanup++;

        if (vm->instns_since_last_cleanup % VM_CLEANUP_PERIOD_INSTNS == 0)
        {
            cleanup_vm(vm);
        }
    }

    cleanup_vm(vm);
}


int vm_fire_events(vm_t* vm)
{
    if (!has_pending_events(vm))
    {
        return -1;
    }

    uint32_t n_events = fire_vm_events(vm);
    if (n_events == 0)
    {
        if (nanosleep(&idle_step, NULL) < 0)
        {
            vm->error_no = errno;
        }
        return 0;
    }

    return n_events;
}


proc_t* vm_current_procedure(vm_t* vm)
{
    proc_t* proc = vm->procedures->data;
    if (proc->iptr >= vm->codesz)
    {
        delete_current_procedure(vm);
        return NULL;
    }
    return proc;
}


int vm_decode_instn(vm_t* vm, proc_t* proc, struct _instn_t* instn)
{
    vm->iptr = proc->iptr;
    uint8_t* iptr = vm->code + proc->iptr;
    int32_t offt = decode_instn(iptr, vm, instn);
    if (offt < 0)
    {
        return -1;
    }

    proc->iptr += offt;
    return 0;
}


int vm_exec_instn(vm_t* vm, instn_t* instn)
{
    uint32_t instn_idx = instn->code >> 3;
    if (InstnDefs[instn_idx].handler(instn, vm) < 0)
    {
        return -1;
    }
    return 0;
}


uint32_t fire_vm_events(vm_t* vm)
{
    return fire_timer_events(vm)
         + fire_io_events(vm)
         + fire_proc_events(vm);
}


int has_pending_events(vm_t* vm)
{
    return has_pending_timer_events(vm)
        || has_pending_io_events(vm)
        || has_pending_proc_events(vm);
}
