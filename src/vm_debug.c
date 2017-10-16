#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "vm.h"
#include "instns/instn.h"
#include "bytecode.h"


void debug_vm(vm_t* vm, vm_debug_data_t* debug_data)
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
