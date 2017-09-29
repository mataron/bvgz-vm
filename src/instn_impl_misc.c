#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "instn.h"
#include "instn_impl.h"


int op_nop_0(instn_t* instn, vm_t* vm)
{
    return 0;
}


int op_set8_2(instn_t* instn, vm_t* vm)
{
    lref8(instn->args[0].ptr) = (uint8_t)arg_value(instn, 1);
    return 0;
}


int op_set16_2(instn_t* instn, vm_t* vm)
{
    lref16(instn->args[0].ptr) = (uint16_t)arg_value(instn, 1);
    return 0;
}


int op_set32_2(instn_t* instn, vm_t* vm)
{
    lref32(instn->args[0].ptr) = (uint32_t)arg_value(instn, 1);
    return 0;
}


int op_set64_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1);
    return 0;
}


int op_cp_3(instn_t* instn, vm_t* vm)
{
    memcpy(instn->args[0].ptr, instn->args[1].ptr, arg_value(instn, 2));
    return 0;
}


int op_deref_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref, vm);
    if (!mem)
    {
        return -1;
    }

    lref64(instn->args[0].ptr) = *(uint64_t*)mem;
    return 0;
}


int op_mexp_1(instn_t* instn, vm_t* vm)
{
    uint64_t sz = arg_value(instn, 0);
    uint8_t* mem = realloc(vm->memory, vm->memsz + sz);
    if (!mem)
    {
        vm->exceptions |= VM_E_OutOfMemory;
        return -1;
    }

    vm->memory = mem;
    vm->memsz += sz;
    return 0;
}


int op_mret_1(instn_t* instn, vm_t* vm)
{
    uint64_t sz = arg_value(instn, 0);
    if (sz > vm->memsz)
    {
        vm->exceptions |= VM_E_MemoryUnderflow;
        return -1;
    }

    vm->memsz -= sz;
    return 0;
}
