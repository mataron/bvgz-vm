#include <stdlib.h>

#include "debug_util.h"
#include "instns/instn.h"


uint32_t parse_uint(char* n, int radix)
{
    char* end = NULL;
    uint32_t value = strtoul(n, &end, radix);
    if (*end) // require the full string to be valid.
    {
        return (uint32_t)-1;
    }

    return value;
}


static uint32_t resolve_address(char* address_or_label, dbg_state_t* state,
    int is_mem_ref)
{
    if (state->labels)
    {
        dbg_label_t* label = NULL;
        int ret = hmap_get(state->labels, address_or_label, (void**)&label);
        if (ret == MAP_OK)
        {
            if (label->is_mem_ref != is_mem_ref)
            {
                return (uint32_t)-1;
            }

            return label->address;
        }
    }

    if (address_or_label[0] == '0' && address_or_label[1] == 'x')
    {
        return parse_uint(address_or_label, 16);
    }

    return parse_uint(address_or_label, 10);
}


uint32_t resolve_mem_address(char* address_or_label, dbg_state_t* state)
{
    return resolve_address(address_or_label, state, 1);
}


uint32_t resolve_code_address(char* address_or_label, dbg_state_t* state)
{
    return resolve_address(address_or_label, state, 0);
}


static const char BrkTypes[] = { 'A', 'L', 'F', 'I', '+' };

void print_breakpoint(dbg_break_pt_t* brk, dbg_state_t* state)
{
    printf("[%3u] %c: ", brk->brk_id, BrkTypes[brk->type - 1]);
    switch (brk->type)
    {
    case BRK_T_Address:
        print_code_address(brk->point.address, state, 0);
        break;
    case BRK_T_Label:
        printf("0x%08x :: %s", brk->point.label.address,
            brk->point.label.name);
        break;
    case BRK_T_Line:
        printf("0x%08x :: %s:%u", brk->point.line.address,
            brk->point.line.file, brk->point.line.lineno);
        break;
    case BRK_T_InstnName:
        printf("[%s]", brk->point.instn_name);
        break;
    case BRK_T_InstnCount:
        printf("[%u]", brk->point.instn_count);
        break;
    }
}


struct label_descr
{
    uint32_t address;
    int is_mem_ref;
    char* label;
};

static void find_label(void* _descr, char* key, void* value)
{
    struct label_descr* descr = _descr;
    dbg_label_t* label = value;
    if (label->is_mem_ref == descr->is_mem_ref &&
        label->address == descr->address)
    {
        descr->label = key;
    }
}


static char* address_label(uint32_t address, dbg_state_t* state,
    int is_mem_ref)
{
    if (state->labels)
    {
        struct label_descr descr = { address, is_mem_ref, NULL };
        hmap_iterate(state->labels, &descr, find_label);
        return descr.label;
    }

    return NULL;
}


int print_mem_address(uint32_t address, dbg_state_t* state)
{
    char* label = address_label(address, state, 1);
    if (label)
    {
        printf(label);
        return 1;
    }

    printf("0x%08x", address);
    return 0;
}


int print_code_address(uint32_t address, dbg_state_t* state, int skip_label)
{
    if (!skip_label)
    {
        char* label = address_label(address, state, 0);
        if (label)
        {
            printf(label);
            return 1;
        }
    }

    int32_t lineno = -1;
    if (state->data)
    {
        for (uint32_t i = 0; i < state->data->n_code_lines; i++)
        {
            dbg_line_assoc_t* code = DBG_CODE_LINE(state->data, i);
            if (code->address == address)
            {
                lineno = code->lineno;
                break;
            }
        }
    }

    if (lineno >= 0)
    {
        printf("0x%08x [%4d]", address, lineno);
        return 0;
    }

    printf("0x%08x", address);
    return 0;
}


void print_instn(instn_t* instn, dbg_state_t* state)
{
    uint32_t instn_idx = instn->code >> 3;

    printf("%8s/%d\t", InstnDefs[instn_idx].name,
        InstnDefs[instn_idx].arg_count);
    for (int i = 0; i < InstnDefs[instn_idx].arg_count; ++i)
    {
        printf("  ");
        if (instn->code & 1 << i)
        {
            int is_mem_ref =
                i == 0 && (InstnDefs[instn_idx].flags & F_1st_Arg_Code_Ref)
                    ? 0 : 1;

            switch(1 << ((instn->arg_sizes >> (2 * i)) & 3))
            {
                case 1:
                    printf("0x%02lX", (uint64_t)instn->args[i].u8);
                    break;
                case 2:
                    printf("0x%04lX", (uint64_t)instn->args[i].u16);
                    break;
                case 4:
                {
                    char *label =
                        address_label(instn->args[i].u32, state, is_mem_ref);
                    if (label)
                    {
                        printf("&%s", label);
                    }
                    else
                    {
                        printf("0x%08lX", (uint64_t)instn->args[i].u32);
                    }
                    break;
                }
                default:
                {
                    if (FITS_IN_32Bit(instn->args[i].u64))
                    {
                        char *label =
                            address_label(instn->args[i].u32, state,
                                is_mem_ref);
                        if (label)
                        {
                            printf("&%s", label);
                            break;
                        }
                    }
                    printf("0x%016lX", instn->args[i].u64);
                }
            }
        }
        else
        {
            uint32_t addr = instn->args[i].ptr - state->vm->memory;
            char *label = address_label(addr, state, 1);
            if (label)
            {
                printf(label);
            }
            else
            {
                printf("*0x%08X", addr);
            }
        }
    }
}

