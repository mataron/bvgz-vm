#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instn.h"


int compare_instn_def(const void* a, const void* b)
{
    char** _a = (char**)a;
    char** _b = (char**)b;
	return strcmp(*_a, *_b);
}


void setup_instn_defs()
{
    int count = 0;
    for (instn_def_t* def = InstnDefs; def->name; def++)
    {
        count++;
    }

    nInstnDefs = count;
    qsort(InstnDefs, nInstnDefs, sizeof(instn_def_t),
        compare_instn_def);

    uint8_t n = 0;
    for (instn_def_t* def = InstnDefs; def->name; def++)
    {
        def->opcode = n++;
    }
}
