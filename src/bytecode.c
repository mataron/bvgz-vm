#include <assert.h>
#include <stdlib.h>

#include "bytecode.h"

int parse_to_bytecode(prs_result_t* parse, void** memory, uint32_t* size)
{
    assert(parse->consistent == 0);
    return 0;
}


int bytecode_to_assembly(FILE* fp, void* memory, uint32_t* size)
{
    return 0;
}
