#ifndef _BVGZ_SYS_PROC_H
#define _BVGZ_SYS_PROC_H

#include <stdint.h>
#include <sys/types.h>

#include "syscall.h"


#define PROC_ALLOC    8


typedef struct _vm_child_t
{
    pid_t pid;
    int used;
    vm_callback_t* exit_cb;
    uint32_t n_exit_cb;
}
vm_child_t;


typedef struct _vm_proc_t
{
    vm_child_t* child_proc;
    uint32_t n_proc;
    uint32_t alloc_proc;

    uint32_t n_exit_callbacks;
}
vm_proc_t;


void destroy_vm_proc(struct _vm_t* vm);

uint32_t fire_proc_events(struct _vm_t* vm);
int has_pending_proc_events(struct _vm_t* vm);

void cleanup_child_proc(struct _vm_t* vm);

#endif
