#ifndef _BVGZZ_INSTN_H
#define _BVGZZ_INSTN_H

#include <stdint.h>
#include "vm.h"

typedef uint16_t    instn_t;

typedef uint32_t (*instn_f) (vm_t*);


typedef struct _instn_def_t {
    instn_t instn;
    const char* name;
    instn_f instn_func;
} instn_def_t;


extern instn_def_t InstnDefs[];

#endif
