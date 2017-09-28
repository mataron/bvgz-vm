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
    qsort(InstnDefs, nInstnDefs, sizeof(instn_def_t), compare_instn_def);

    uint8_t n = 0;
    for (instn_def_t* def = InstnDefs; def->name; def++)
    {
        def->opcode = n++;
    }
}


int nInstnDefs = 0;

// handler defintions:
#define OP(x, n) extern int op_ ## x ## _ ## n(struct _instn_t* instn, struct _vm_t* vm);
#define OP0(x) OP(x, 0)
#define OP1(x) OP(x, 1)
#define OP2(x) OP(x, 2)
#define OP3(x) OP(x, 3)
#define OP1f(x, f) OP1(x)
#define OP2f(x, f) OP2(x)
#define OP3f(x, f) OP3(x)
#include "instn_defs.inc"
#undef OP
#undef OP0
#undef OP1
#undef OP2
#undef OP3
#undef OP1f
#undef OP2f
#undef OP3f


// instn definitions:
#define OP(x, n, f) { #x, 0, n, f, NULL },
#define OP0(x) OP(x, 0, 0)
#define OP1(x) OP(x, 1, 0)
#define OP2(x) OP(x, 2, 0)
#define OP3(x) OP(x, 3, 0)
#define OP1f(x, f) OP(x, 1, f)
#define OP2f(x, f) OP(x, 2, f)
#define OP3f(x, f) OP(x, 3, f)

instn_def_t InstnDefs[] = {
#include "instn_defs.inc"
    { NULL }
};

#undef OP
#undef OP0
#undef OP1
#undef OP2
#undef OP3
#undef OP1f
#undef OP2f
#undef OP3f
