#include <stdio.h>
#include "instn.h"


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
#define OP(x, n, f) { #x, 0, n, f, op_ ## x ## _ ## n },
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
