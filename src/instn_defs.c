#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instn.h"

#define OP(x, n, f) { #x, 0, n, f }
#define OP0(x) OP(x, 0, 0)
#define OP1(x) OP(x, 1, 0)
#define OP2(x) OP(x, 2, 0)
#define OP3(x) OP(x, 3, 0)
#define OP1f(x, f) OP(x, 1, f)
#define OP2f(x, f) OP(x, 2, f)
#define OP3f(x, f) OP(x, 3, f)


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
        def->opcode = count;
        count++;
    }
    nInstnDefs = count;
    qsort(InstnDefs, nInstnDefs, sizeof(instn_def_t), compare_instn_def);
}


int nInstnDefs = 0;

instn_def_t InstnDefs[] = {
    OP0(nop),
    OP3(add),
    OP2(add),
    OP3(sub),
    OP2(sub),
    OP3(mul),
    OP2(mul),
    OP3(div),
    OP2(div),
    OP3(mod),
    OP2(mod),
    OP3(eq),
    OP3(ne),
    OP3(gt),
    OP3(lt),
    OP3(ge),
    OP3(le),
    OP3(l_and),
    OP2(l_and),
    OP3(l_or),
    OP2(l_or),
    OP2(l_not),
    OP1(l_not),
    OP2(l_bool),
    OP1(l_bool),
    OP3(and),
    OP2(and),
    OP3(or),
    OP2(or),
    OP3(xor),
    OP2(xor),
    OP2(not),
    OP1(not),
    OP3(shl),
    OP2(shl),
    OP3(shr),
    OP2(shr),
    OP2(set8),
    OP2(set16),
    OP2(set32),
    OP2(set64),
    OP1f(jmp, F_ALLOW_IMM_1st_Arg),
    OP2f(jtrue, F_ALLOW_IMM_1st_Arg),
    OP2f(jfalse, F_ALLOW_IMM_1st_Arg),
    OP3f(call, F_ALLOW_IMM_1st_Arg),
    OP0(ret),
    OP1f(mexp, F_ALLOW_IMM_1st_Arg),
    OP1f(mret, F_ALLOW_IMM_1st_Arg),
    OP0(yield),
    { NULL, 0, 0 }
};
