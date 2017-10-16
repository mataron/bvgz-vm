#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"
#include "instns/instn.h"


static int nCommands = 0;


static void setup_commands();
static void print_init_message(dbg_state_t* state);
static char* read_command();
static char** split_cmdline(char* cmdline, int* argc);
static int exec_command(int argc, char** argv, dbg_state_t* state);


void debug_vm(vm_t* vm, vm_debug_data_t* debug_data)
{
    dbg_state_t state;
    state.vm = vm;
    state.data = debug_data;
    state.flags = 0;

    setup_commands();

    print_init_message(&state);
    while (1)
    {
        char* cmd = read_command();
        if (!cmd)
        {
            break;
        }

        int argc = 0;
        char** argv = split_cmdline(cmd, &argc);
        if (!argv)
        {
            free(cmd);
            break;
        }

        if (exec_command(argc, argv, &state) < 0)
        {
            break;
        }

        free(cmd);
        free(argv);
    }

    cleanup_vm(vm);
}


static int compare_dbg_commands(const void* a, const void* b)
{
    char** _a = (char**)a;
    char** _b = (char**)b;
	return strcmp(*_a, *_b);
}


static void setup_commands()
{
    for (dbg_command_t* cmd = Commands; cmd->name; cmd++)
    {
        nCommands++;
    }

    qsort(Commands, nCommands, sizeof(dbg_command_t),
        compare_dbg_commands);
}


static void print_init_message(dbg_state_t* state)
{
    printf("BVGZ VM Debugger\n");
    printf("Code: %u bytes | Mem: %u bytes\n",
        state->vm->codesz, state->vm->memsz);
    printf("Code lines: %u\n", state->data->n_code_lines);
    printf("Mem symbols: %u\n", state->data->n_mem_lines);
    printf("Labels: %u\n", state->data->n_labels);
    printf("Files: %u\n", state->data->n_files);
}


static char* read_command()
{
    char* cmd = NULL;
    char* buf = NULL;
    size_t length = 0;

    while (1)
    {
        buf = NULL;
        length = 0;

        printf("> ");

        size_t len = getline(&buf, &length, stdin);
        if (len < 0)
        {
            return NULL;
        }

        // trim:

        for (cmd = buf; *cmd; cmd++)
        {
            if (!isspace(*cmd)) break;
            *cmd = 0;
        }

        for (char* c = buf + len - 1; c > buf; c--)
        {
            if (isspace(*c)) *c = 0;
            else break;
        }

        len = strlen(cmd);
        if (len > 0)
        {
            break;
        }
    }

    cmd = strdup(cmd);
    free(buf);

    return cmd;
}


static char** split_cmdline(char* cmdline, int* argc)
{
    char** argv = malloc(sizeof(char*));
    argv[0] = cmdline;
    *argc = 1;

    int reached_arg = 0;
    char* c = cmdline;
    while (*c)
    {
        if (isspace(*c))
        {
            *c = 0;
            reached_arg = 1;
            c++;
            continue;
        }

        if (reached_arg)
        {
            (*argc)++;
            argv = realloc(argv, sizeof(char*) * *argc);
            argv[*argc - 1] = c;
            reached_arg = 0;
        }

        c++;
    }

    return argv;
}


static int exec_command(int argc, char** argv, dbg_state_t* state)
{
    dbg_command_t* cmd;

    cmd = bsearch(argv, Commands, nCommands,
        sizeof(dbg_command_t), compare_dbg_commands);
    if (!cmd)
    {
        printf("Unknown command: [%s]\n", argv[0]);
        return 0;
    }

    return cmd->handler(argc, argv, state);
}
