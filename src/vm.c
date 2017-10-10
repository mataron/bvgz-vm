#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "instns/instn.h"
#include "syscalls/syscall.h"

int verbose = 0;
int collect_stats = 0;

#define STACK_FREE_THRESHOLD    ((uint32_t)(1.5 * FUNC_STACK_ALLOC_SZ))


void initialize_engine()
{
    setup_instn_defs();
    setup_system_call_table();
}


vm_t* make_vm(uint32_t codesz, uint32_t memsz, uint32_t entry)
{
    vm_t* vm = malloc(sizeof(vm_t));
    memset(vm, 0, sizeof(vm_t));
    vm->codesz = codesz;
    vm->memsz = memsz;
    vm->code = malloc(codesz);
    if (memsz)
    {
        vm->memory = malloc(memsz);
        memset(vm->memory, 0, memsz);
    }
    make_procedure(entry, vm);
    vm->iptr = (uint32_t)-1;
    return vm;
}


void destroy_vm(vm_t* vm)
{
    while (vm->procedures)
    {
        delete_current_procedure(vm);
    }

    destroy_vm_io(vm);
    free(vm->memory);
    free(vm->code);
    free(vm);
}


proc_t* make_procedure(uint32_t iptr, vm_t* vm)
{
    if (iptr >= vm->codesz)
    {
        vm->exceptions |= VM_E_BadInstnPointer;
        return NULL;
    }

    proc_t* proc = malloc(sizeof(proc_t));
    proc->iptr = iptr;
    proc->cstack = NULL;
    proc->cstack_sz = 0;
    proc->cstack_alloc = 0;

    list_t* proc_node = list_make_node(proc);

    if (!vm->procedures)
    {
        vm->procedures = proc_node;
    }
    else
    {
        list_t* tail = list_tail(vm->procedures);
        list_append(tail, proc_node);
    }

    return proc;
}


proc_t* make_func_procedure(uint32_t iptr, uint32_t argv,
    uint32_t retv, vm_t* vm)
{
    proc_t* proc = make_procedure(iptr, vm);
    if (proc)
    {
        push_call_stack(proc, retv, argv, (uint32_t)-1, vm);
    }
    return proc;
}


void push_call_stack(proc_t* proc, uint32_t retval,
    uint32_t args, uint32_t ret_address, vm_t* vm)
{
    if (proc->cstack_alloc < proc->cstack_sz + 1)
    {
        proc->cstack_alloc += FUNC_STACK_ALLOC_SZ;
        proc->cstack = realloc(proc->cstack,
            sizeof(fcall_t) * proc->cstack_alloc);
    }

    proc->cstack[proc->cstack_sz].retval = retval;
    proc->cstack[proc->cstack_sz].args = args;
    proc->cstack[proc->cstack_sz].ret_address = ret_address;
    proc->cstack_sz++;
}


uint32_t pop_call_stack(proc_t* proc, vm_t* vm)
{
    proc->cstack_sz--;

    fcall_t* top = &proc->cstack[proc->cstack_sz];
    uint32_t ret_address = top->ret_address;

    uint32_t excess = proc->cstack_alloc - proc->cstack_sz;
    if (excess >= STACK_FREE_THRESHOLD)
    {
        proc->cstack_alloc -= FUNC_STACK_ALLOC_SZ;
        proc->cstack = realloc(proc->cstack, proc->cstack_alloc);
    }

    return ret_address;
}


void delete_current_procedure(vm_t* vm)
{
    list_t* current = vm->procedures;
    vm->procedures = current->next;

    proc_t* proc = current->data;
    free(proc->cstack);
    free(proc);

    list_destroy_node(current);
}


void make_vm_timer(vm_t* vm, struct timespec* exprires_at,
    uint32_t iptr)
{
    vm->timers = realloc(vm->timers,
        sizeof(vm_timer_t) * (vm->n_timers + 1));

    vm->timers[vm->n_timers].expires_at = *exprires_at;
    vm->timers[vm->n_timers].iptr = iptr;
    vm->n_timers++;
}


void print_vm_state(vm_t* vm)
{
    char* state = "RUNNING";
    if (vm->exceptions)
    {
        state = "ERROR";
    }
    else if (!vm->procedures)
    {
        state = "STOPPED";
    }
    printf("VM State: %s\n", state);
    printf("Code size: %u | Memory: %u\n", vm->codesz, vm->memsz);
    printf("  Last instn addr: 0x%08x\n", vm->iptr);
    printf("  Procedures: %u\n",
        vm->procedures ? list_size(vm->procedures) : 0);
    printf("  Instns run: %lu\n", vm->instns);
    if (vm->error_no)
    {
        printf("  Last error: %s\n", strerror(vm->error_no));
    }
    if (vm->exceptions)
    {
        printf("Exceptions:\n");
        if (vm->exceptions & VM_E_Arithmetic)
            printf("  Arithmetic\n");
        if (vm->exceptions & VM_E_MemFault)
            printf("  Mem fault\n");
        if (vm->exceptions & VM_E_OutOfMemory)
            printf("  Out of memory\n");
        if (vm->exceptions & VM_E_MemoryUnderflow)
            printf("  Memory underflow\n");
        if (vm->exceptions & VM_E_BadInstnPointer)
            printf("  Bad Instn ptr\n");
        if (vm->exceptions & VM_E_BadInstnCode)
            printf("  Bad Instn\n");
        if (vm->exceptions & VM_E_BadSyscallNo)
            printf("  Bad syscall\n");
    }
}
