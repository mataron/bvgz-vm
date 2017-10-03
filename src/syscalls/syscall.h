#ifndef _BVGZ_SYSCALL_H
#define _BVGZ_SYSCALL_H

#include <stdint.h>

struct _vm_t;
typedef
int (*syscall_f)(struct _vm_t* vm, uint32_t argv, uint32_t retv);

extern syscall_f SyscallTable[];
extern int nSyscalls;

void setup_system_call_table();

#endif
