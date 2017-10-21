#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"
#include "instns/instn.h"


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
    if (!state->data)
    {
        printf("No symbols available\n");
        return 0;
    }

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


int dbg_show_vm(int argc, char** argv, dbg_state_t* state)
{
    if (!state->flags) printf("VM Stopped\n");
    else if (state->flags & F_RUNNING) printf("VM Running\n");

    printf("Code sz: %5u | Mem sz: %u\n",
        state->vm->codesz, state->vm->memsz);

    printf("Instns run: %4lu | Since last cleanup: %u\n",
        state->vm->instns, state->vm->instns_since_last_cleanup);
    printf("Events: Tmrs:%3u | I/O fds:%2u | I/O events:%2u"
        " | Procs:%2u | Procs C/Bs:%2u\n", state->vm->n_timers,
        state->vm->io.used_fds, state->vm->io.n_io_events,
        state->vm->proc.n_proc, state->vm->proc.n_exit_callbacks);
    printf("Last error: %s\n", strerror(state->vm->error_no));

    if (state->vm->exceptions)
    {
        printf("Exceptions:\n");
        print_exception("  ", state->vm->exceptions);
    }

    printf("Runnable procedures: %u\n", list_size(state->vm->procedures));
    printf("Next instn: 0x%08x\n", state->vm->iptr);

    return 0;
}


int dbg_dump_byte(int argc, char** argv, dbg_state_t* state);
int dbg_dump_word(int argc, char** argv, dbg_state_t* state);
int dbg_dump_dword(int argc, char** argv, dbg_state_t* state);
int dbg_dump_qword(int argc, char** argv, dbg_state_t* state);
int dbg_dump_str(int argc, char** argv, dbg_state_t* state);

int dbg_break(int argc, char** argv, dbg_state_t* state);
int dbg_show_breakpts(int argc, char** argv, dbg_state_t* state);
int dbg_break_delete(int argc, char** argv, dbg_state_t* state);

int dbg_run(int argc, char** argv, dbg_state_t* state);
int dbg_step(int argc, char** argv, dbg_state_t* state);

dbg_command_t Commands[] = {
    { "q", "quit the debugger", dbg_quit },
    { "help", "show this message", dbg_help },
    { "sym", "show symbols: sym [c|m]", dbg_show_symbols },
    { "vm", "show vm state", dbg_show_vm },
    { "db", "dump byte", dbg_dump_byte },
    { "dw", "dump word", dbg_dump_word },
    { "dd", "dump doubleword", dbg_dump_dword },
    { "dq", "dump quadword", dbg_dump_qword },
    { "ds", "dump string", dbg_dump_str },
    { "brk", "set breakpoint", dbg_break },
    { "brks", "show breakpoints", dbg_show_breakpts },
    { "brkd", "delete breakpoint", dbg_break_delete },
    { "run", "run the program", dbg_run },
    { "n", "execute next instn", dbg_step },
    { NULL, NULL, NULL }
};
