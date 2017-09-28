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


int op_jmp_1(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_jtrue_2(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_jfalse_2(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_call_3(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_syscall_3(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_ret_0(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_mexp_1(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_mret_1(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}


int op_yield_0(instn_t* instn, vm_t* vm)
{
    // TODO: implement
    return 0;
}
