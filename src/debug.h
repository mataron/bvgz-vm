#ifndef _BVGZ_DEBUG_H
#define _BVGZ_DEBUG_H

#include "vm.h"
#include "bytecode.h"
#include "util/hashmap.h"


typedef struct _dbg_label_t
{
    char* label;
    int is_mem_ref;
    uint32_t address;
}
dbg_label_t;


#define F_RUNNING   0x1

typedef struct _dbg_state_t
{
    vm_t* vm;
    vm_debug_data_t* data;
    unsigned flags;

    // values are dbg_label_t
    hashmap_t labels;

    char* help_format_string;
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
