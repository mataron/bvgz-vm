#ifndef _BVGZ_SYSCALL_H
#define _BVGZ_SYSCALL_H

#include <stdint.h>

struct _vm_t;
typedef
void (*syscall_f)(struct _vm_t* vm, uint32_t argv, uint32_t retv);

extern syscall_f SyscallTable[];
extern int nSyscalls;

void setup_system_call_table();

extern char* BVGZ_image_gen_dir;

uint8_t* deref(uint32_t ref, uint32_t size, struct _vm_t* vm);
int ensure_nul_term_str(uint8_t* base, struct _vm_t* vm);

#define FITS_IN_32Bit(v64)  (((v64) >> 32) == 0)

#endif
