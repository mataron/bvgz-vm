#include "debug.h"


int dbg_quit(int argc, char** argv, dbg_state_t* state)
{
    printf("Exiting...\n");
    return -1;
}


dbg_command_t Commands[] = {
    { "q", "quit the debugger", dbg_quit },
    { NULL, NULL, NULL }
};
