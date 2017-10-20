#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "debug_util.h"


uint32_t parse_uint(char* n, int radix)
{
    char* end = NULL;
    uint32_t value = strtoul(n, &end, radix);
    if (*end) // require the full string to be valid.
    {
        return (uint32_t)-1;
    }

    return value;
}


uint32_t resolve_mem_address(char* address_or_label, dbg_state_t* state)
{
    dbg_label_t* label = NULL;
    int ret = hmap_get(state->labels, address_or_label, (void**)&label);
    if (ret == MAP_OK)
    {
        if (!label->is_mem_ref)
        {
            return (uint32_t)-1;
        }

        return label->address;
    }

    if (address_or_label[0] == '0' && address_or_label[1] == 'x')
    {
        return parse_uint(address_or_label, 16);
    }

    return parse_uint(address_or_label, 10);
}
