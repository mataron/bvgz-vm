#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "instn.h"
#include "instn_impl.h"


int op_nop_0(instn_t* instn, vm_t* vm)
{
    return 0;
}


int op_assert_2(instn_t* instn, vm_t* vm)
{
    if (arg_value(instn, 1))
    {
        vm->assert_id = arg_value(instn, 0);
        vm->exceptions |= VM_E_AssertFailed;
        return -1;
    }
    return 0;
}


int op_abrt_1(instn_t* instn, vm_t* vm)
{
    vm->exceptions |= arg_value(instn, 0);
    return -1;
}


int op_cp_3(instn_t* instn, vm_t* vm)
{
    memcpy(instn->args[0].ptr, instn->args[1].ptr, arg_value(instn, 2));
    return 0;
}


int op_read8_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[1].ptr);
    uint32_t offset = arg_value(instn, 2);
    uint8_t* mem = deref_mem_ptr(ref + offset, 1, vm);
    if (!mem)
    {
        return -1;
    }

    lref8(instn->args[0].ptr) = *mem;
    return 0;
}


int op_read16_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[1].ptr);
    uint32_t offset = arg_value(instn, 2);
    uint8_t* mem = deref_mem_ptr(ref + offset, 2, vm);
    if (!mem)
    {
        return -1;
    }

    lref16(instn->args[0].ptr) = *(uint16_t*)mem;
    return 0;
}


int op_read32_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[1].ptr);
    uint32_t offset = arg_value(instn, 2);
    uint8_t* mem = deref_mem_ptr(ref + offset, 4, vm);
    if (!mem)
    {
        return -1;
    }

    lref32(instn->args[0].ptr) = *(uint32_t*)mem;
    return 0;
}


int op_read64_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[1].ptr);
    uint32_t offset = arg_value(instn, 2);
    uint8_t* mem = deref_mem_ptr(ref + offset, 8, vm);
    if (!mem)
    {
        return -1;
    }

    lref64(instn->args[0].ptr) = *(uint64_t*)mem;
    return 0;
}


int op_write8_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = arg_value(instn, 0);
    uint32_t offset = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref + offset, 1, vm);
    if (!mem)
    {
        return -1;
    }

    *mem = arg_value(instn, 2);
    return 0;
}


int op_write16_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = arg_value(instn, 0);
    uint32_t offset = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref + offset, 2, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint16_t*)mem = arg_value(instn, 2);
    return 0;
}


int op_write32_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = arg_value(instn, 0);
    uint32_t offset = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref + offset, 4, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint32_t*)mem = arg_value(instn, 2);
    return 0;
}


int op_write64_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = arg_value(instn, 0);
    uint32_t offset = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref + offset, 8, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint64_t*)mem = arg_value(instn, 2);
    return 0;
}
