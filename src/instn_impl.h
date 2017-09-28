#include <stdint.h>
#include "bytecode.h"


#define lref(r, bits)   *(uint ## bits ## _t*)(r)
#define lref64(r)       lref(r, 64)
#define lref32(r)       lref(r, 32)
#define lref16(r)       lref(r, 16)
#define lref8(r)        lref(r, 8)


static inline uint64_t arg_value(instn_t* instn, int index)
{
    if (instn->code & 1 << index)
    {
        switch(1 << ((instn->arg_sizes >> (2 * index)) & 3))
        {
            case 1: return (uint64_t) instn->args[index].u8;
            case 2: return (uint64_t) instn->args[index].u16;
            case 4: return (uint64_t) instn->args[index].u32;
        }
        return instn->args[index].u64;
    }

    return lref64(instn->args[index].ptr);
}
