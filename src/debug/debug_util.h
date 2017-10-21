#ifndef _BVGZ_DEBUG_UTIL_H
#define _BVGZ_DEBUG_UTIL_H

#include "debug.h"

uint32_t parse_uint(char* n, int radix);

uint32_t resolve_mem_address(char* address_or_label, dbg_state_t* state);

void print_breakpoint(dbg_break_pt_t* brk);

struct _instn_t;
void print_instn(struct _instn_t* instn, dbg_state_t* state);

#endif
