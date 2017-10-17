#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"


int dbg_quit(int argc, char** argv, dbg_state_t* state)
{
    printf("Exiting...\n");
    return -1;
}


int dbg_help(int argc, char** argv, dbg_state_t* state)
{
    if (!state->help_format_string)
    {
        int mxlen = 0;
        for (dbg_command_t* c = Commands; c->name; c++)
        {
            int len = strlen(c->name);
            if (len > mxlen) mxlen = len;
        }

        state->help_format_string = malloc(20);
        snprintf(state->help_format_string, 19, "%%%ds\t%%s\n", mxlen + 1);
    }

    for (dbg_command_t* c = Commands; c->name; c++)
    {
        printf(state->help_format_string, c->name, c->description);
    }

    return 0;
}


int dbg_show_symbols(int argc, char** argv, dbg_state_t* state)
{
    if (argc == 1 || argv[1][0] != 'c')
    {
        printf("Memory symbols (%u):\n", state->data->n_mem_lines);
        for (uint32_t m = 0; m < state->data->n_mem_lines; m++)
        {
            dbg_line_assoc_t* mem = DBG_MEM_LINE(state->data, m);
            printf("  0x%08x:  %s\n", mem->address,
                DBG_LABEL(state->data, mem->label_ref));
        }
    }
    if (argc == 1 || argv[1][0] != 'm')
    {
        uint32_t n_code_labels = 0;
        for (uint32_t c = 0; c < state->data->n_code_lines; c++)
        {
            if (DBG_CODE_LINE(state->data, c)->label_ref >= 0)
            {
                n_code_labels++;
            }
        }

        printf("Code symbols (%u):\n", n_code_labels);
        for (uint32_t c = 0; c < state->data->n_code_lines; c++)
        {
            dbg_line_assoc_t* code = DBG_CODE_LINE(state->data, c);
            if (code->label_ref >= 0)
            {
                printf("  0x%08x:  %s\n", code->address,
                    DBG_LABEL(state->data, code->label_ref));
            }
        }
    }
    return 0;
}


dbg_command_t Commands[] = {
    { "q", "quit the debugger", dbg_quit },
    { "help", "show this message", dbg_help },
    { "sym", "show symbols: sym [c|m]", dbg_show_symbols },
    { NULL, NULL, NULL }
};
