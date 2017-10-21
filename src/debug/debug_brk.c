#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug_util.h"
#include "instns/instn.h"


static dbg_break_pt_t* create_and_attach_breakpoint(dbg_state_t* state,
    unsigned type)
{
    dbg_break_pt_t* bpt = malloc(sizeof(dbg_break_pt_t));

    bpt->type = type;
    bpt->brk_id = state->brk_id_pool++;

    list_t* node = list_make_node(bpt);
    list_prepend(state->breakpoints, node);
    state->breakpoints = node;

    return bpt;
}


int dbg_break(int argc, char** argv, dbg_state_t* state)
{
    if (argc != 2)
    {
        printf("Usage: %s [label|address|:line|file:line|+instns|instn]\n",
            argv[0]);
        return 0;
    }


    if (argv[1][0] == '+')
    {
        uint32_t count = parse_uint(argv[1] + 1, 10);
        if (count == (uint32_t)-1)
        {
            printf("invalid instruction count: [%s]\n", argv[1] + 1);
            return 0;
        }

        dbg_break_pt_t* bpt =
            create_and_attach_breakpoint(state, BRK_T_InstnCount);
        bpt->point.instn_count = state->vm->instns + count;
        return 0;
    }

    char* colon = state->data ? strchr(argv[1], ':') : NULL;
    if (colon)
    {
        *colon = 0;
        char* file = argv[1];
        uint32_t lineno = parse_uint(colon + 1, 10);
        if (lineno == (uint32_t)-1)
        {
            printf("bad line number: [%s]\n", colon + 1);
            return 0;
        }

        for (uint32_t c = 0; c < state->data->n_code_lines; c++)
        {
            dbg_line_assoc_t* code = DBG_CODE_LINE(state->data, c);
            char* c_file = DBG_FILE(state->data, code->fileno);
            if ((!*file || !strcmp(file, c_file)) && lineno == code->lineno)
            {
                dbg_break_pt_t* bpt =
                    create_and_attach_breakpoint(state, BRK_T_Line);
                bpt->point.line.address = code->address;
                bpt->point.line.file = c_file;
                bpt->point.line.lineno = lineno;
                return 0;
            }
        }
    }

    if (state->labels)
    {
        dbg_label_t* label = NULL;
        int ret = hmap_get(state->labels, argv[1], (void**)&label);
        if (ret == MAP_OK)
        {
            if (label->is_mem_ref)
            {
                printf("label [%s] points into data segment\n", argv[1]);
                return 0;
            }

            dbg_break_pt_t* bpt =
                create_and_attach_breakpoint(state, BRK_T_Label);
            bpt->point.label.name = label->label;
            bpt->point.label.address = label->address;
            return 0;
        }
    }

    instn_def_t* fmt = bsearch(&argv[1], InstnDefs, nInstnDefs,
        sizeof(instn_def_t), compare_instn_def);
    if (fmt)
    {
        dbg_break_pt_t* bpt =
            create_and_attach_breakpoint(state, BRK_T_InstnName);
        bpt->point.instn_name = (char*)fmt->name;
        return 0;
    }

    if (argv[1][0] == '0' && argv[1][1] == 'x')
    {
        uint32_t address = parse_uint(argv[1], 16);
        if (address != (uint32_t)-1)
        {
            dbg_break_pt_t* bpt =
                create_and_attach_breakpoint(state, BRK_T_Address);
            bpt->point.address = address;
            return 0;
        }
    }

    uint32_t address = parse_uint(argv[1], 10);
    if (address != (uint32_t)-1)
    {
        dbg_break_pt_t* bpt =
            create_and_attach_breakpoint(state, BRK_T_Address);
        bpt->point.address = address;
        return 0;
    }

    printf("cannot parse breakpoint: [%s]\n", argv[1]);
    return 0;
}


int dbg_show_breakpts(int argc, char** argv, dbg_state_t* state)
{
    uint32_t sz = list_size(state->breakpoints);
    printf("Breakpoints (%u):\n", sz);

    for (list_t* n = state->breakpoints; n; n = n->next)
    {
        print_breakpoint(n->data);
        printf("\n");
    }

    return 0;
}


int dbg_break_delete(int argc, char** argv, dbg_state_t* state)
{
    if (argc != 2)
    {
        printf("Usage: %s [brk id]\n",
            argv[0]);
        return 0;
    }

    uint32_t brkid = parse_uint(argv[1], 10);
    if (brkid == (uint32_t)-1)
    {
        printf("invalid breakpoint id: [%s]\n", argv[1]);
        return 0;
    }

    for (list_t* n = state->breakpoints; n; n = n->next)
    {
        dbg_break_pt_t* brk = n->data;
        if (brk->brk_id == brkid)
        {
            if (state->breakpoints == n)
            {
                state->breakpoints = n->next;
            }
            list_unlink(n);
            free(brk);
            free(n);
            return 0;
        }
    }

    printf("breakpoint not found: [%u]\n", brkid);
    return 0;
}
