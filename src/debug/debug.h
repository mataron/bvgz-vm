#ifndef _BVGZ_DEBUG_H
#define _BVGZ_DEBUG_H

#include "vm.h"
#include "bytecode.h"
#include "util/hashmap.h"
#include "util/list.h"


typedef struct _dbg_label_t
{
    char* label;
    int is_mem_ref;
    uint32_t address;
}
dbg_label_t;


#define BRK_T_Address       1
#define BRK_T_Label         2
#define BRK_T_Line          3
#define BRK_T_InstnName     4
#define BRK_T_InstnCount    5

typedef struct _dbg_break_pt_t
{
    unsigned type;
    union
    {
        uint32_t address;
        struct
        {
            char* name;
            uint32_t address;
        }
        label;
        struct
        {
            char* file;
            uint32_t lineno;
            uint32_t address;
        }
        line;
        char* instn_name;
        uint32_t instn_count;
    }
    point;
    uint32_t brk_id;
}
dbg_break_pt_t;


#define F_RUNNING   0x1

typedef struct _dbg_state_t
{
    vm_t* vm;
    vm_debug_data_t* data;
    unsigned flags;

    // values are dbg_label_t
    hashmap_t labels;
    // values are dbg_break_pt_t
    list_t* breakpoints;
    uint32_t brk_id_pool;

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
