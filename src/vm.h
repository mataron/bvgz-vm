#ifndef _BVGZ_VM_H
#define _BVGZ_VM_H

#include <stdint.h>
#include "util/list.h"


#define VM_E_Arithmetic     0x1
#define VM_E_MemFault       0x2


typedef struct _vm_t
{
    uint8_t* code;
    uint32_t codesz;

    uint8_t* memory;
    uint32_t memsz;

    unsigned flags;
    unsigned exceptions;

    // procedures: instruction pointers into 'code'
    list_t* procedures;
}
vm_t;

#endif
