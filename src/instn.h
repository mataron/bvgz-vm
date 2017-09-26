#ifndef _BVGZZ_INSTN_H
#define _BVGZZ_INSTN_H

#include <stdint.h>


typedef struct _instn_def_t
{
    const char* name;
    uint16_t opcode;
    int arg_count;
}
instn_def_t;


extern instn_def_t InstnDefs[];
extern int nInstnDefs;

void setup_instn_defs();

#endif
