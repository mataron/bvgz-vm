#ifndef _BVGZ_DEBUG_UTIL_H
#define _BVGZ_DEBUG_UTIL_H

#include "debug.h"

uint32_t parse_uint(char* n, int radix);

uint32_t resolve_mem_address(char* address_or_label, dbg_state_t* state);

#endif
