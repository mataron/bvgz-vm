#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bytecode.h"
#include "parser.h"
#include "instn.h"

#define BUF_BLOCK_SIZE  256

struct ref
{
    uint32_t offset;
    uint32_t instn;
};


static uint32_t compute_layout(prs_instn_t* instn, instn_def_t* def,
    uint16_t* OpIR, uint8_t* ISz);
static uint8_t nlog2(uint8_t n);
static uint32_t write_args(prs_instn_t* instn, instn_def_t* def,
    uint8_t* buffer, uint32_t offset, prs_result_t* parse,
    uint32_t cur_instn, struct ref* delayed_refs, int* n_delayed_refs);
static uint32_t write_num_ref_arg(uint8_t* buffer, uint32_t offset,
    prs_arg_t* arg);
static uint32_t write_imm_arg(uint8_t* buffer, uint32_t offset,
    prs_arg_t* arg);
static uint32_t write_imm32bit(uint8_t* buffer, uint32_t offset,
    uint32_t value);
static struct ref* add_delayed_ref(struct ref* refs, int n_refs,
    uint32_t offset, uint32_t instn);


int parse_to_bytecode(prs_result_t* parse, uint8_t** memory,
    uint32_t* size)
{
    assert(parse->consistent == 0);
    uint8_t* buffer = NULL;
    uint32_t n_blocks = 0;
    uint32_t offset = 0;
    struct ref* delayed_refs = NULL;
    int n_delayed_refs = 0;

    for (uint32_t i = 0; i < parse->n_instns; i++)
    {
        prs_instn_t* instn = parse->instns[i];
        instn_def_t* def = instn->instn;

        uint16_t OpIR = 0;
        uint8_t ISz = 0;
        uint32_t size = compute_layout(instn, def, &OpIR, &ISz);

        while (offset + size > n_blocks * BUF_BLOCK_SIZE)
        {
            n_blocks++;
            buffer = realloc(buffer, n_blocks * BUF_BLOCK_SIZE);
        }

        instn->mem_offset = offset;

        *(uint16_t*)(buffer + offset) = OpIR;
        offset += 2;
        if (OpIR & 0x7) buffer[offset++] = ISz;

        offset = write_args(instn, def, buffer, offset, parse, i,
            delayed_refs, &n_delayed_refs);
    }

    for (int i = 0; i < n_delayed_refs; i++)
    {
        uint32_t ref = parse->instns[delayed_refs[i].instn]->mem_offset;
        write_imm32bit(buffer, delayed_refs[i].offset, ref);
    }

    free(delayed_refs);
    *memory = buffer;
    *size = offset;
    return 0;
}


static uint32_t compute_layout(prs_instn_t* instn, instn_def_t* def,
    uint16_t* OpIR, uint8_t* ISz)
{
    uint32_t size = 1;
    *OpIR = def->opcode << 3;
    for (uint32_t j = 0; j < def->arg_count; j++)
    {
        if (instn->args[j].type == T_ARG_IMM)
        {
            *OpIR |= 1 << j;
            *ISz |= nlog2(instn->args[j].n_bytes) << (2 * j);
        }
        size += instn->args[j].n_bytes;
    }
    if (*OpIR & 0x7) size++;

    return size;
}


static uint8_t nlog2(uint8_t n)
{
    switch(n)
    {
    case 1: return 0;
    case 2: return 1;
    case 4: return 2;
    }
    return 3;
}


static uint32_t write_args(prs_instn_t* instn, instn_def_t* def,
    uint8_t* buffer, uint32_t offset, prs_result_t* parse,
    uint32_t cur_instn, struct ref* delayed_refs, int* n_delayed_refs)
{
    for (uint32_t j = 0; j < def->arg_count; j++)
    {
        switch(instn->args[j].type)
        {
        case T_ARG_REF_NUM:
            offset = write_num_ref_arg(buffer, offset,
                &instn->args[j]);
            break;

        case T_ARG_IMM:
            offset = write_imm_arg(buffer, offset, &instn->args[j]);
            break;

        case T_ARG_REF_LBL:
        {
            label_t* label = NULL;
            hmap_get(parse->labels, instn->args[j].data.label,
                (void**)&label);

            if (label->is_mem_ref)
            {
                offset = write_imm32bit(buffer, offset, label->offset);
                break;
            }

            uint64_t ref_instn_index = label->offset;
            if (ref_instn_index <= cur_instn)
            {
                offset = write_imm32bit(buffer, offset,
                    parse->instns[ref_instn_index]->mem_offset);
            }
            else
            {
                delayed_refs = add_delayed_ref(delayed_refs,
                    *n_delayed_refs, offset, ref_instn_index);
                (*n_delayed_refs)++;
                offset += 4;
            }
        }
        }
    }

    return offset;
}


static uint32_t write_num_ref_arg(uint8_t* buffer, uint32_t offset,
    prs_arg_t* arg)
{
    *(uint32_t*)(buffer + offset) =
        (uint32_t)(0xffffffff & arg->data.value);
    return offset + 4;
}


static uint32_t write_imm_arg(uint8_t* buffer, uint32_t offset,
    prs_arg_t* arg)
{
    switch(arg->n_bytes)
    {
    case 1:
        buffer[offset] = (uint8_t)(0xff & arg->data.value);
        return offset + 1;

    case 2:
        *(uint16_t*)(buffer + offset) =
            (uint16_t)(0xffff & arg->data.value);
        return offset + 2;

    case 4:
        return write_imm32bit(buffer, offset,
            (uint32_t)(0xffffffff & arg->data.value));
    }

    *(uint64_t*)(buffer + offset) = arg->data.value;
    return offset + 8;
}


static uint32_t write_imm32bit(uint8_t* buffer, uint32_t offset,
    uint32_t value)
{
    *(uint32_t*)(buffer + offset) = value;
    return offset + 4;
}


static struct ref* add_delayed_ref(struct ref* refs, int n_refs,
    uint32_t offset, uint32_t instn)
{
    refs = realloc(refs, sizeof(struct ref) * (1 + n_refs));
    refs[n_refs].offset = offset;
    refs[n_refs].instn = instn;
    return refs;
}


int write_bvgz_image(FILE *fp, struct _prs_result_t* parse,
    uint8_t* code, uint32_t codesz, uint32_t entry_label)
{
    uint16_t magic = BVGZ_IMG_MAGIC;
    if (fwrite(&magic, sizeof(uint16_t), 1, fp) != sizeof(uint16_t))
    {
        fprintf(stderr, "fwrite(magic): %s\n", strerror(errno));
        return -1;
    }

    uint16_t flags = BVGZ_IMG_F_EXEC;
    if (fwrite(&flags, sizeof(uint16_t), 1, fp) != sizeof(uint16_t))
    {
        fprintf(stderr, "fwrite(flags): %s\n", strerror(errno));
        return -1;
    }

    if (fwrite(&entry_label, sizeof(uint32_t), 1, fp) !=
        sizeof(uint32_t))
    {
        fprintf(stderr, "fwrite(entry): %s\n", strerror(errno));
        return -1;
    }

    if (fwrite(&codesz, sizeof(uint32_t), 1, fp) != sizeof(uint32_t))
    {
        fprintf(stderr, "fwrite(codesz): %s\n", strerror(errno));
        return -1;
    }

    if (fwrite(&parse->memsz, sizeof(uint32_t), 1, fp) !=
        sizeof(uint32_t))
    {
        fprintf(stderr, "fwrite(memsz): %s\n", strerror(errno));
        return -1;
    }

    if (fwrite(code, codesz, 1, fp) != codesz)
    {
        fprintf(stderr, "fwrite(code:%u): %s\n",
            codesz, strerror(errno));
        return -1;
    }

    if (fwrite(parse->memory, parse->memsz, 1, fp) != parse->memsz)
    {
        fprintf(stderr, "fwrite(mem:%u): %s\n",
            parse->memsz, strerror(errno));
        return -1;
    }

    return 0;
}
