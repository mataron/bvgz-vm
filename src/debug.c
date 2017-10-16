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


dbg_command_t Commands[] = {
    { "q", "quit the debugger", dbg_quit },
    { "help", "show this message", dbg_help },
    { NULL, NULL, NULL }
};
