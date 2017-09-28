#include "vm.h"
#include "instn.h"
#include "instn_impl.h"


int op_eq_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) == arg_value(instn, 2) ? 1 : 0;
    return 0;
}


int op_ne_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) != arg_value(instn, 2) ? 1 : 0;
    return 0;
}


int op_gt_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) > arg_value(instn, 2) ? 1 : 0;
    return 0;
}


int op_lt_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) < arg_value(instn, 2) ? 1 : 0;
    return 0;
}


int op_ge_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) >= arg_value(instn, 2) ? 1 : 0;
    return 0;
}


int op_le_3(instn_t* instn, vm_t* vm)
{
    lref64(instn->args[0].ptr) = arg_value(instn, 1) <= arg_value(instn, 2) ? 1 : 0;
    return 0;
}
