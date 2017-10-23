#ifndef _BVgZ_PARSER_H
#define _BVgZ_PARSER_H

#include <stdint.h>

#include "util/list.h"
#include "util/hashmap.h"


#define T_ARG_IMM       0
#define T_ARG_REF_NUM   1
#define T_ARG_REF_LBL   2
#define T_ARG_IMM_LBL   3

typedef struct _prs_arg_t
{
    uint8_t type;
    uint8_t n_bytes;   // size in bytes of the immediate value
    union {
        uint64_t value;
        char* label;
    } data;
}
prs_arg_t;


struct _instn_def_t;
typedef struct _prs_instn_t
{
    const char* filename;
    uint32_t lineno;

    struct _instn_def_t* instn;
    prs_arg_t* args;
    // set by the bytecode generator: offset in memory
    // where this instruction is found.
    uint32_t mem_offset;
}
prs_instn_t;


typedef struct _label_t
{
    const char* filename;
    uint32_t lineno;

    uint32_t offset;
    // when false, offset refers to an instn.
    uint8_t is_mem_ref;
}
label_t;


typedef struct _label_ref_t
{
    const char* filename;
    uint32_t lineno;

    const char* label;
    uint32_t size;
    uint32_t write_offset;
}
label_ref_t;


typedef struct _prs_result_t
{
    uint32_t n_files;

    uint32_t errors;
    uint32_t warnings;

    uint32_t n_instns;
    prs_instn_t** instns;
    // memory area built from data
    uint8_t* memory;
    uint32_t memsz; // bytes in 'memory' that are used
    uint32_t mem_alloc; // allocation size of 'memory'
    hashmap_t labels;
    // stores label references in the data section
    // these are set after parse.
    list_t* label_refs;
    // when zero everything is fine!
    int consistent;
}
prs_result_t;


prs_result_t* parse_asm(char* filename, list_t* include_paths);

int resolve_data_label_refs(prs_result_t* result);

uint32_t resolve_entry_point(char* entry_label, prs_result_t* result);

void destroy_parse_result(prs_result_t* parse_result);

#endif
