#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "vm.h"
#include "instns/instn.h"
#include "bytecode.h"


#define F_RUNNING   0x1

typedef struct _dbg_state_t
{
    vm_t* vm;
    vm_debug_data_t* data;
    unsigned flags;
}
dbg_state_t;


static void print_init_message(dbg_state_t* state);
static char* read_command();
static int exec_command(char* cmdline, dbg_state_t* state);


void debug_vm(vm_t* vm, vm_debug_data_t* debug_data)
{
    dbg_state_t state;
    state.vm = vm;
    state.data = debug_data;
    state.flags = 0;

    print_init_message(&state);
    while (1)
    {
        char* cmd = read_command();
        if (!cmd)
        {
            break;
        }

        if (exec_command(cmd, &state) < 0)
        {
            break;
        }
    }

    cleanup_vm(vm);
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
    return NULL;
}


static int exec_command(char* cmdline, dbg_state_t* state)
{
    return -1;
}
