#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "vm.h"
#include "instns/instn.h"
#include "bytecode.h"

#define IDLE_STEP_DURATION_MS   10

// TODO: stats collection



#undef ENABLE_INSTN_PRINT
// #define ENABLE_INSTN_PRINT

#ifdef ENABLE_INSTN_PRINT
static void print_instn(instn_t* instn);
#endif


void execute_vm(vm_t* vm)
{
    struct timespec idle_step = {
        MILLISECONDS_TO_SECONDS(IDLE_STEP_DURATION_MS),
        MILLISECONDS_TO_NANOSECONDS(IDLE_STEP_DURATION_MS)
    };
    instn_t instn;

    while (1)
    {
        if (!vm->procedures)
        {
            if (!vm->n_timers)
            {
                break;
            }
            if (fire_vm_events(vm) == 0)
            {
                if (nanosleep(&idle_step, NULL) < 0)
                {
                    vm->error_no = errno;
                }
                continue;
            }
        }

        proc_t* proc = vm->procedures->data;
        if (proc->iptr >= vm->codesz)
        {
            delete_current_procedure(vm);
            continue;
        }

        vm->iptr = proc->iptr;
        uint8_t* iptr = vm->code + proc->iptr;
        int32_t offt = decode_instn(iptr, vm, &instn);
        if (offt < 0)
        {
            break;
        }

        proc->iptr += offt;

#ifdef ENABLE_INSTN_PRINT
        print_instn(&instn);
#endif
        uint32_t instn_idx = instn.code >> 3;
        if (InstnDefs[instn_idx].handler(&instn, vm) < 0)
        {
            break;
        }

        vm->instns++;
    }
}


uint32_t fire_vm_events(vm_t* vm)
{
    return fire_timer_events(vm);
}


#ifdef ENABLE_INSTN_PRINT
static void print_instn(instn_t* instn)
{
    uint32_t instn_idx = instn->code >> 3;

    printf("%s/%d\t", InstnDefs[instn_idx].name,
        InstnDefs[instn_idx].arg_count);
    for (int i = 0; i < InstnDefs[instn_idx].arg_count; ++i)
    {
        if (instn->code & 1 << i)
        {
            switch(1 << ((instn->arg_sizes >> (2 * i)) & 3))
            {
                case 1:
                    printf(" 0x%02lX", (uint64_t)instn->args[i].u8);
                    break;
                case 2:
                    printf(" 0x%04lX", (uint64_t)instn->args[i].u16);
                    break;
                case 4:
                    printf(" 0x%08lX", (uint64_t)instn->args[i].u32);
                    break;
            }
            printf(" 0x%016lX", instn->args[i].u64);
        }
        else
        {
            printf(" *0x%016lX", (uint64_t)instn->args[i].ptr);
        }
    }
    printf("\n");
}
#endif
