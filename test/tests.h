#ifndef _BVGZ_TESTS_H
#define _BVGZ_TESTS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define xstr(s) str(s)
#define str(s) #s

struct _vm_t;
struct _vm_t* mk_vm_for_asm(char* asmfile);

#endif
