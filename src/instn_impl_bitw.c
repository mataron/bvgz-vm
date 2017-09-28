#include "vm.h"
#include "instn.h"
#include "instn_impl.h"


int op_and_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) & arg_value(instn, 2);
    return 0;
}


int op_and_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) &= arg_value(instn, 1);
    return 0;
}


int op_or_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) | arg_value(instn, 2);
    return 0;
}


int op_or_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) |= arg_value(instn, 1);
    return 0;
}


int op_xor_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) ^ arg_value(instn, 2);
    return 0;
}


int op_xor_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) ^= arg_value(instn, 1);
    return 0;
}


int op_not_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = ~arg_value(instn, 1);
    return 0;
}


int op_not_1(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = lref64(instn->args[0].ptr);
    return 0;
}


int op_shl_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) << arg_value(instn, 2);
    return 0;
}


int op_shl_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) <<= arg_value(instn, 1);
    return 0;
}


int op_shr_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) >> arg_value(instn, 2);
    return 0;
}


int op_shr_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) >>= arg_value(instn, 1);
    return 0;
}
