#ifndef _BVGZ_DEBUG_H
#define _BVGZ_DEBUG_H

#include "vm.h"
#include "bytecode.h"


#define F_RUNNING   0x1

typedef struct _dbg_state_t
{
    vm_t* vm;
    vm_debug_data_t* data;
    unsigned flags;
}
dbg_state_t;



typedef int (*cmd_f)(int, char**, dbg_state_t*);

typedef struct _dbg_command_t
{
    const char* name;
    const char* description;

    cmd_f handler;
}
dbg_command_t;


extern dbg_command_t Commands[];

#endif
