#ifndef _BVGZ_BYTECODE_H
#define _BVGZ_BYTECODE_H

#include <stdio.h>
#include <stdint.h>

#define BVGZ_IMG_MAGIC      0xB0C5
#define BVGZ_IMG_F_EXEC     0x1

struct _prs_result_t;

// ASSUMES the 'parse' is correct & internally consistent.
int parse_to_bytecode(struct _prs_result_t* parse,
    uint8_t** memory, uint32_t* size);
int bytecode_to_text(FILE* fp, void* memory, uint32_t* size);


typedef struct _instn_t
{
    uint16_t code;
    uint8_t arg_sizes;
    union
    {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        uint8_t* ptr;
    }
    args[3];
}
instn_t;

struct _vm_t;
int decode_instn(uint8_t* iptr, struct _vm_t* vm, instn_t* instn);

uint8_t* deref_mem_ptr(uint32_t ref, struct _vm_t* vm);

int write_bvgz_image(FILE *fp, struct _prs_result_t* parse,
    uint8_t* code, uint32_t codesz, uint32_t entry_label);
struct _vm_t* read_bvgz_image(FILE *fp);

#endif
