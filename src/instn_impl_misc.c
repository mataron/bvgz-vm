#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "instn.h"
#include "instn_impl.h"


int op_nop_0(instn_t* instn, vm_t* vm)
{
    return 0;
}


int op_cp8_2(instn_t* instn, vm_t* vm)
{
    lref8(instn->args[0].ptr) = (uint8_t)arg_value(instn, 1);
    return 0;
}


int op_cp16_2(instn_t* instn, vm_t* vm)
{
    lref16(instn->args[0].ptr) = (uint16_t)arg_value(instn, 1);
    return 0;
}


int op_cp32_2(instn_t* instn, vm_t* vm)
{
    lref32(instn->args[0].ptr) = (uint32_t)arg_value(instn, 1);
    return 0;
}


int op_cp64_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1);
    return 0;
}


int op_cp_3(instn_t* instn, vm_t* vm)
{
    memcpy(instn->args[0].ptr, instn->args[1].ptr, arg_value(instn, 2));
    return 0;
}


int op_read8_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[1].ptr);
    uint8_t* mem = deref_mem_ptr(ref, 1, vm);
    if (!mem)
    {
        return -1;
    }

    lref8(instn->args[0].ptr) = *mem;
    return 0;
}


int op_read16_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[1].ptr);
    uint8_t* mem = deref_mem_ptr(ref, 2, vm);
    if (!mem)
    {
        return -1;
    }

    lref16(instn->args[0].ptr) = *(uint16_t*)mem;
    return 0;
}


int op_read32_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[1].ptr);
    uint8_t* mem = deref_mem_ptr(ref, 4, vm);
    if (!mem)
    {
        return -1;
    }

    lref32(instn->args[0].ptr) = *(uint32_t*)mem;
    return 0;
}


int op_read64_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[1].ptr);
    uint8_t* mem = deref_mem_ptr(ref, 8, vm);
    if (!mem)
    {
        return -1;
    }

    lref64(instn->args[0].ptr) = *(uint64_t*)mem;
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


int op_write8_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[0].ptr);
    uint8_t* mem = deref_mem_ptr(ref, 1, vm);
    if (!mem)
    {
        return -1;
    }

    *mem = lref8(instn->args[1].ptr);
    return 0;
}


int op_write16_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[0].ptr);
    uint8_t* mem = deref_mem_ptr(ref, 2, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint16_t*)mem = lref16(instn->args[1].ptr);
    return 0;
}


int op_write32_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[0].ptr);
    uint8_t* mem = deref_mem_ptr(ref, 4, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint32_t*)mem = lref32(instn->args[1].ptr);
    return 0;
}


int op_write64_2(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[0].ptr);
    uint8_t* mem = deref_mem_ptr(ref, 8, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint64_t*)mem = lref64(instn->args[1].ptr);
    return 0;
}


int op_write8_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[0].ptr);
    uint32_t offset = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref + offset, 1, vm);
    if (!mem)
    {
        return -1;
    }

    *mem = lref8(instn->args[2].ptr);
    return 0;
}


int op_write16_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[0].ptr);
    uint32_t offset = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref + offset, 2, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint16_t*)mem = lref16(instn->args[2].ptr);
    return 0;
}


int op_write32_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[0].ptr);
    uint32_t offset = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref + offset, 4, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint32_t*)mem = lref32(instn->args[2].ptr);
    return 0;
}


int op_write64_3(instn_t* instn, vm_t* vm)
{
    uint32_t ref = lref32(instn->args[0].ptr);
    uint32_t offset = arg_value(instn, 1);
    uint8_t* mem = deref_mem_ptr(ref + offset, 8, vm);
    if (!mem)
    {
        return -1;
    }

    *(uint64_t*)mem = lref64(instn->args[2].ptr);
    return 0;
}
