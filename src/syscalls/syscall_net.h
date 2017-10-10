#ifndef _BVGZ_SYS_NET_H
#define _BVGZ_SYS_NET_H

#include <netinet/in.h>

#include "syscall_io.h"


#define SOCK_T_TCP  1
#define SOCK_T_UDP  2

#define LISTEN_BACKLOG  10


#define NET_SOCK_ACCEPTING  0x1
#define NET_SOCK_CONNECTING 0x2
#define NET_SOCK_CONNECTED  0x4


typedef struct _vm_net_sock_t
{
    struct sockaddr_in addr;

    uint16_t flags;
}
vm_net_sock_t;


typedef struct _vm_net_connect_t
{
    uint32_t callback;
    uint32_t args;
}
vm_net_connect_t;


#endif
