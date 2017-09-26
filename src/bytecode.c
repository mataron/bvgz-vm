#include "bytecode.h"


int32_t decode_instn(uint8_t* iptr, uint8_t* memory, instn_t* instn)
{
    int32_t offset = 0;
    uint8_t instn_idx = *iptr >> 3;
    if (instn_idx > nInstnDefs)
    {
        return -1;
    }

    instn->code = *(uint16_t*)iptr;
    offset += 2;
    if (instn->code & 0x7)
    {
        instn->arg_sizes = *(iptr + offset);
        offset++;
    }

    for (int i = 0; i < InstnDefs[instn_idx].arg_count; i++)
    {
        if (instn->code & (1 << i))
        {
            int n_bytes = 1 << ((instn->arg_sizes >> (2 * i)) & 3);
            switch (n_bytes)
            {
            case 1:
                instn->args[i].u8 = *(iptr + offset);
                offset++;
                break;
            case 2:
                instn->args[i].u16 = *(uint16_t*)(iptr + offset);
                offset += 2;
                break;
            case 4:
                instn->args[i].u32 = *(uint32_t*)(iptr + offset);
                offset += 4;
                break;
            default:
                instn->args[i].u64 = *(uint64_t*)(iptr + offset);
                offset += 8;
                break;
            }
        } else {
            instn->args[i].ptr = memory + *(uint32_t*)(iptr + offset);
            offset += 4;
        }
    }

    return offset;
}
