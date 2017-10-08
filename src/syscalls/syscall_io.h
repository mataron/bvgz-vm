#ifndef _BVGZ_SYS_IO_H
#define _BVGZ_SYS_IO_H

#include <stdint.h>

#include "syscall.h"


#define FD_T_FILE       0x1
#define FD_T_NET_TCPv4  0x2
#define FD_T_NET_UDPv4  0x4

#define IO_BUF_ALLOC    4


typedef struct _io_mem_t
{
    uint32_t ptr;
    uint32_t len;
    uint32_t callback;
    uint32_t args;
}
io_mem_t;


typedef struct _vm_fd_t
{
    uint8_t type;
    uint8_t used;
    uint64_t pos;
    int fd;

    io_mem_t* read_bufs;
    uint32_t n_read_bufs;
    uint32_t alloc_read_bufs;

    io_mem_t* write_bufs;
    uint32_t n_write_bufs;
    uint32_t alloc_write_bufs;
}
vm_fd_t;


typedef struct _vm_io_t
{
    vm_fd_t* fds;
    uint32_t n_fds;
    uint32_t used_fds;

    uint32_t n_io_bufs;
}
vm_io_t;


struct _vm_t;
uint64_t alloc_fd(struct _vm_t* vm);
void dealloc_fd(uint64_t fd, struct _vm_t* vm);

uint32_t fire_io_events(struct _vm_t* vm);
int has_pending_io_events(struct _vm_t* vm);

#define FD_HANDLE_TO_IDX(arg)  ((arg) - 1)
#define FD_IDX_TO_HANDLE(arg)  ((arg) + 1)

#endif
