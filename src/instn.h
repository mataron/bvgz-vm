#ifndef _BVGZZ_INSTN_H
#define _BVGZZ_INSTN_H

#include <stdint.h>

#define F_ALLOW_IMM_1st_Arg     0x1

typedef struct _instn_def_t
{
    const char* name;
    uint16_t opcode;
    int arg_count;
    unsigned flags;
}
instn_def_t;


extern instn_def_t InstnDefs[];
extern int nInstnDefs;

// works with both instn_def_t AND char** arguments
int compare_instn_def(const void*, const void*);

void setup_instn_defs();

#endif
