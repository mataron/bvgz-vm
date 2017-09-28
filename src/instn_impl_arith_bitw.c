#include "vm.h"
#include "instn.h"
#include "instn_impl.h"


int op_add_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) + arg_value(instn, 2);
    return 0;
}


int op_add_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) += arg_value(instn, 1);
    return 0;
}


int op_sub_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) - arg_value(instn, 2);
    return 0;
}


int op_sub_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) -= arg_value(instn, 1);
    return 0;
}


int op_mul_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) * arg_value(instn, 2);
    return 0;
}


int op_mul_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) *= arg_value(instn, 1);
    return 0;
}


int op_div_3(instn_t* instn, vm_t* vm)
{
    uint64_t divisor = arg_value(instn, 2);
    if (divisor == 0)
    {
        vm->exceptions |= VM_E_Arithmetic;
        return -1;
    }

    lref64(instn->args[0].ptr) = arg_value(instn, 1) / divisor;
    return 0;
}


int op_div_2(instn_t* instn, vm_t* vm)
{
    uint64_t divisor = arg_value(instn, 1);
    if (divisor == 0)
    {
        vm->exceptions |= VM_E_Arithmetic;
        return -1;
    }

    lref64(instn->args[0].ptr) /= divisor;
    return 0;
}


int op_mod_3(instn_t* instn, vm_t* vm)
{
    uint64_t divisor = arg_value(instn, 2);
    if (divisor == 0)
    {
        vm->exceptions |= VM_E_Arithmetic;
        return -1;
    }

    lref64(instn->args[0].ptr) = arg_value(instn, 1) % divisor;
    return 0;
}


int op_mod_2(instn_t* instn, vm_t* vm)
{
    uint64_t divisor = arg_value(instn, 1);
    if (divisor == 0)
    {
        vm->exceptions |= VM_E_Arithmetic;
        return -1;
    }

    lref64(instn->args[0].ptr) %= divisor;
    return 0;
}


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
