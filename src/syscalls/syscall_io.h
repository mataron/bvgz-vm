#ifndef _BVGZ_SYS_IO_H
#define _BVGZ_SYS_IO_H

#include <stdint.h>

#include "syscall.h"


#define IO_EVT_ALLOC    4

typedef struct _io_mem_t
{
    uint32_t ptr;
    uint32_t len;
    uint32_t callback;
    uint32_t args;
}
io_mem_t;


struct _vmt_t;
struct _vm_fd_t;
struct _vm_io_evt_t;
typedef uint32_t (*vm_io_evt_handler_t)
    (struct _vm_t*, struct _vm_fd_t*, struct _vm_io_evt_t*);


#define IO_EVT_SELECT_READ    0x1
#define IO_EVT_SELECT_WRITE   0x2
    

typedef struct _vm_io_evt_t
{
    unsigned flags: 2;
    vm_io_evt_handler_t activate;
    void* data;
}
vm_io_evt_t;


typedef struct _vm_fd_t
{
    int fd;
    unsigned used: 1;

    vm_io_evt_t* events;
    uint32_t n_events;
    uint32_t alloc_events;
}
vm_fd_t;


typedef struct _vm_io_t
{
    vm_fd_t* fds;
    uint32_t n_fds;
    uint32_t used_fds;

    uint32_t n_io_events;
}
vm_io_t;


void destroy_vm_io(struct _vm_t* vm);

uint64_t alloc_fd(struct _vm_t* vm);
void dealloc_fd(uint64_t fd, struct _vm_t* vm);

uint32_t fire_io_events(struct _vm_t* vm);
int has_pending_io_events(struct _vm_t* vm);

#define FD_HANDLE_TO_IDX(arg)  ((arg) - 1)
#define FD_IDX_TO_HANDLE(arg)  ((arg) + 1)

#endif
