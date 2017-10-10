#ifndef _BVGZ_SYS_NET_H
#define _BVGZ_SYS_NET_H

#include <netinet/in.h>

#include "syscall_io.h"


#define SOCK_T_TCP  1
#define SOCK_T_UDP  2

#define LISTEN_BACKLOG  10


typedef struct _vm_net_sock_t
{
    struct sockaddr_in addr;
}
vm_net_sock_t;


typedef struct _vm_net_cb_t
{
    uint32_t callback;
    uint32_t args;
}
vm_net_cb_t;


#endif
