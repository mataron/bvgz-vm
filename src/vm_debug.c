#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "debug/debug.h"
#include "instns/instn.h"


static int nCommands = 0;

static char** ac_labels = NULL; // used by completion function
static int n_ac_labels = 0;
static int ac_prefer_labels = 0;

static void setup_commands();
static void print_init_message(dbg_state_t* state);
static char* read_command();
static char** split_cmdline(char* cmdline, int* argc);
static int exec_command(int argc, char** argv, dbg_state_t* state);


static char* cmdline_generator(const char* text, int state)
{
    static int cmd_index, len;
    const char *name;

    if (!state) {
        cmd_index = 0;
        len = strlen(text);
    }

    if (ac_prefer_labels && ac_labels)
    {
        while (cmd_index < n_ac_labels)
        {
            name = ac_labels[cmd_index];
            cmd_index++;

            if (strncmp(name, text, len) == 0)
            {
                return strdup(name);
            }
        }
    }
    else
    {
        while (cmd_index < nCommands)
        {
            name = Commands[cmd_index].name;
            cmd_index++;

            if (strncmp(name, text, len) == 0)
            {
                return strdup(name);
            }
        }
    }

    return NULL;
}

static char** cmdline_completion(const char* text, int start, int end)
{
    ac_prefer_labels = start > 0;
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, cmdline_generator);
}


static void setup_state(dbg_state_t* state, vm_t* vm,
    vm_debug_data_t* debug_data)
{
    memset(state, 0, sizeof(dbg_state_t));

    rl_attempted_completion_function = cmdline_completion;

    state->vm = vm;
    state->data = debug_data;

    ac_labels = NULL;
    n_ac_labels = 0;

    if (state->data)
    {
        state->labels = hmap_create();
        for (uint32_t m = 0; m < state->data->n_mem_lines; m++)
        {
            dbg_line_assoc_t* mem = DBG_MEM_LINE(state->data, m);
            char* str = DBG_LABEL(state->data, mem->label_ref);
            dbg_label_t* lbl = malloc(sizeof(dbg_label_t));
            lbl->label = str;
            lbl->address = mem->address;
            lbl->is_mem_ref = 1;
            hmap_put(state->labels, str, lbl);

            n_ac_labels++;
            ac_labels = realloc(ac_labels, sizeof(char*) * n_ac_labels);
            ac_labels[n_ac_labels - 1] = str;
        }
        for (uint32_t c = 0; c < state->data->n_code_lines; c++)
        {
            dbg_line_assoc_t* code = DBG_CODE_LINE(state->data, c);
            if (code->label_ref >= 0)
            {
                char* str = DBG_LABEL(state->data, code->label_ref);
                dbg_label_t* lbl = malloc(sizeof(dbg_label_t));
                lbl->label = str;
                lbl->address = code->address;
                lbl->is_mem_ref = 0;
                hmap_put(state->labels, str, lbl);

                n_ac_labels++;
                ac_labels = realloc(ac_labels, sizeof(char*) * n_ac_labels);
                ac_labels[n_ac_labels - 1] = str;
            }
        }
    }
}


static void free_label(void* unused, char* key, void* value)
{
    free(value);
}


static void cleanup_state(dbg_state_t* state)
{
    if (state->breakpoints)
    {
        for (list_t* b = state->breakpoints; b; b = b->next)
        {
            free(b->data);
        }
        list_destroy(state->breakpoints);
    }

    if (state->labels)
    {
        hmap_iterate(state->labels, NULL, free_label);
        hmap_destroy(state->labels);
    }

    free(state->help_format_string);
    cleanup_vm(state->vm);

    free(ac_labels);
}


void debug_vm(vm_t* vm, vm_debug_data_t* debug_data)
{
    dbg_state_t state;
    setup_state(&state, vm, debug_data);

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

        int ret = exec_command(argc, argv, &state);

        free(cmd);
        free(argv);

        if (ret < 0)
        {
            break;
        }
    }

    cleanup_state(&state);
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
    if (state->data)
    {
        printf("Code lines: %u\n", state->data->n_code_lines);
        printf("Mem symbols: %u\n", state->data->n_mem_lines);
        printf("Labels: %u\n", state->data->n_labels);
        printf("Files: %u\n", state->data->n_files);
    }
    else
    {
        printf("No debug symbols available\n");
    }
    printf("Type 'help' for a list of available commands\n");
}


static char* read_command()
{
    char* cmd = NULL;
    char* buf = NULL;

    while (1)
    {
        buf = readline("> ");
        if (!buf)
        {
            return NULL;
        }

        int len = strlen(buf);

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

        add_history(cmd);
        len = strlen(cmd);
        if (len > 0)
        {
            break;
        }

        free(buf);
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
