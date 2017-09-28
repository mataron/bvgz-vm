#include "vm.h"
#include "instn.h"
#include "instn_impl.h"


int op_l_and_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) && arg_value(instn, 2) ? 1 : 0;
    return 0;
}


int op_l_and_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = lref64(instn->args[0].ptr) && arg_value(instn, 1) ? 1 : 0;
    return 0;
}


int op_l_or_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) || arg_value(instn, 2) ? 1 : 0;
    return 0;
}


int op_l_or_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = lref64(instn->args[0].ptr) || arg_value(instn, 1) ? 1 : 0;
    return 0;
}


int op_l_not_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) ? 0 : 1;
    return 0;
}


int op_l_not_1(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = lref64(instn->args[0].ptr) ? 0 : 1;
    return 0;
}


int op_l_bool_2(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) ? 1 : 0;
    return 0;
}


int op_l_bool_1(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = lref64(instn->args[0].ptr) ? 1 : 0;
    return 0;
}
