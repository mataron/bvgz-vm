#include "vm.h"
#include "instn.h"
#include "bytecode.h"

// TODO: stats collection

void execute_vm(vm_t* vm)
{
    instn_t instn;

    while (1)
    {
        if (!vm->procedures)
        {
            break;
        }

        proc_t* proc = vm->procedures->data;
        vm->iptr = proc->iptr;
        uint8_t* iptr = vm->code + proc->iptr;
        int32_t offt = decode_instn(iptr, vm, &instn);
        if (offt < 0)
        {
            break;
        }

        proc->iptr += offt;

        uint32_t instn_idx = instn.code >> 3;
        if (InstnDefs[instn_idx].handler(&instn, vm) < 0)
        {
            break;
        }

        vm->instns++;
    }
}
