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


struct _instn_t;
struct _vm_t;
int32_t decode_instn(uint8_t* iptr, struct _vm_t* vm,
    struct _instn_t* instn);

uint8_t* deref_mem_ptr(uint32_t ref, uint32_t size, struct _vm_t* vm);
int ensure_nul_term_str(uint8_t* base, struct _vm_t* vm);

int write_bvgz_image(FILE *fp, struct _prs_result_t* parse,
    uint8_t* code, uint32_t codesz, uint32_t entry_label);
struct _vm_t* read_bvgz_image(FILE *fp);

#endif
