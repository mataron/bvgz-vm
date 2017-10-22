#ifndef _BVGZ_BYTECODE_H
#define _BVGZ_BYTECODE_H

#include <stdio.h>
#include <stdint.h>

#define BVGZ_IMG_MAGIC      0xB0C5
#define BVGZ_IMG_F_EXEC     0x1
#define BVGZ_IMG_F_DEBUG    0x2

struct _prs_result_t;

// ASSUMES the 'parse' is correct & internally consistent.
int parse_to_bytecode(struct _prs_result_t* parse,
    uint8_t** memory, uint32_t* size, int debug_mode);
int bytecode_to_text(FILE* fp, void* memory, uint32_t* size);


struct _instn_t;
struct _vm_t;
int32_t decode_instn(uint8_t* iptr, struct _vm_t* vm,
    struct _instn_t* instn);

uint8_t* deref_mem_ptr(uint32_t ref, uint32_t size, struct _vm_t* vm);

int write_bvgz_image(FILE *fp, struct _prs_result_t* parse,
    uint8_t* code, uint32_t codesz, uint32_t entry_label, int debug);
int write_bvgz_image_direct(FILE *fp, uint8_t* code, uint32_t codesz,
    uint8_t* mem, uint32_t memsz, uint32_t entry_label, uint16_t flags);
int write_bvgz_image_debug(FILE *fp, struct _prs_result_t* parse);

struct _vm_t* read_bvgz_image(FILE *fp);

typedef struct _vm_debug_data_t
{
    uint32_t n_code_lines;
    uint32_t n_mem_lines;
    uint32_t n_labels;
    uint32_t n_files;
    uint32_t strtab_sz;

    uint8_t data[0];
}
vm_debug_data_t;

vm_debug_data_t* read_bvgz_debug_data(FILE* fp);


typedef struct
{
    uint32_t address;
    uint32_t fileno;
    uint32_t lineno;
    int32_t label_ref;
}
dbg_line_assoc_t;


#define DBG_CODE_LINE(dbg, n)   \
    ( (dbg_line_assoc_t*) ( \
        (((void*)dbg) + sizeof(vm_debug_data_t)) + \
        sizeof(dbg_line_assoc_t) * n) \
    )

#define DBG_MEM_LINE(dbg, n)   \
    ( (dbg_line_assoc_t*) ( \
        (((void*)dbg) + sizeof(vm_debug_data_t)) + \
        sizeof(dbg_line_assoc_t) * \
            (dbg->n_code_lines + n)) \
    )

#define DBG_LABEL_OFFSET(dbg, n)   \
    ( *(uint32_t*) ( \
        (((void*)dbg) + sizeof(vm_debug_data_t)) + \
        sizeof(dbg_line_assoc_t) * \
            (dbg->n_code_lines + dbg->n_mem_lines) + \
        sizeof(uint32_t) * n) \
    )

#define DBG_FILE_OFFSET(dbg, n)   \
    ( *(uint32_t*) ( \
        (((void*)dbg) + sizeof(vm_debug_data_t)) + \
        sizeof(dbg_line_assoc_t) * \
            (dbg->n_code_lines + dbg->n_mem_lines) + \
        sizeof(uint32_t) * (dbg->n_labels + n)) \
    )

#define DBG_STRTAB_OFFSET(dbg)   \
    ( (char*) ( \
        (((void*)dbg) + sizeof(vm_debug_data_t)) + \
        sizeof(dbg_line_assoc_t) * \
            (dbg->n_code_lines + dbg->n_mem_lines) + \
        sizeof(uint32_t) * \
            (dbg->n_labels + dbg->n_files) ) \
    )

#define DBG_LABEL(dbg, n) \
    ( (char*) ( \
        DBG_STRTAB_OFFSET(dbg) + DBG_LABEL_OFFSET(dbg, n) \
    ) )

#define DBG_FILE(dbg, n) \
    ( (char*) ( \
        DBG_STRTAB_OFFSET(dbg) + DBG_FILE_OFFSET(dbg, n) \
    ) )

#endif
