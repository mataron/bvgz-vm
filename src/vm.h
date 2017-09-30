#ifndef _BVGZ_VM_H
#define _BVGZ_VM_H

#include <stdint.h>
#include "util/list.h"

#define FUNC_STACK_ALLOC_SZ     64

#define VM_E_Arithmetic         0x1
#define VM_E_MemFault           0x2
#define VM_E_OutOfMemory        0x4
#define VM_E_MemoryUnderflow    0x8
#define VM_E_BadInstnPointer    0x10
#define VM_E_BadInstnCode       0x20


typedef struct _fcall_t
{
    uint32_t retval;
    uint32_t args;
    uint32_t ret_address;
}
fcall_t;


typedef struct _proc_t
{
    // pointer to next instruction.
    uint32_t iptr;

    // call stack
    fcall_t* cstack;
    uint32_t cstack_sz;
    uint32_t cstack_alloc;
}
proc_t;


typedef struct _vm_t
{
    uint8_t* code;
    uint32_t codesz;

    uint8_t* memory;
    uint32_t memsz;

    unsigned flags;
    unsigned exceptions;

    // list of runnable procedures (proc_t)
    list_t* procedures;
    // currently executing instruction address
    uint32_t iptr;

    // number of instructions executed so far.
    uint64_t instns;
}
vm_t;


extern int verbose;
extern int collect_stats;


vm_t* make_vm(uint32_t codesz, uint32_t memsz, uint32_t entry);
void destroy_vm(vm_t* vm);

void execute_vm(vm_t* vm);

void print_vm_state(vm_t* vm);

proc_t* make_procedure(uint32_t iptr, vm_t* vm);
proc_t* make_func_procedure(uint32_t iptr, uint32_t argv,
    uint32_t retv, vm_t* vm);

void push_call_stack(proc_t* proc, uint32_t retval,
    uint32_t args, uint32_t ret_address, vm_t* vm);
uint32_t pop_call_stack(proc_t* proc, vm_t* vm);

void delete_current_procedure(vm_t* vm);

#endif
