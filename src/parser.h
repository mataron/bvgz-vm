#ifndef _BVgZ_PARSER_H
#define _BVgZ_PARSER_H

#include <stdint.h>

#include "util/list.h"
#include "util/hashmap.h"
#include "instn.h"


#define T_ARG_IMM       0
#define T_ARG_REF_NUM   1
#define T_ARG_REF_LBL   2

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


typedef struct _prs_instn_t
{
    instn_def_t* instn;
    prs_arg_t* args;
}
prs_instn_t;


typedef struct _prs_result_t
{
    uint32_t n_instns;
    prs_instn_t** instns;
    // maps label names to instn in the array above.
    hashmap_t labels;
}
prs_result_t;


prs_result_t* parse_asm(char* filename, list_t* include_paths);

void destroy_parse_result(prs_result_t* parse_result);

#endif
