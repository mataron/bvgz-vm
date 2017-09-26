#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instn.h"

#define OP(x, n) { #x, 0, n }
#define OP0(x) OP(x, 0)
#define OP1(x) OP(x, 1)
#define OP2(x) OP(x, 2)
#define OP3(x) OP(x, 3)


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
    OP1(jmp),
    OP2(jtrue),
    OP2(jfalse),
    OP3(call),
    OP0(ret),
    OP1(mexp),
    OP1(mret),
    OP0(yield),
    { NULL, 0, 0 }
};
