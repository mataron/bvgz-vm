#ifndef _BVGZ_BYTECODE_H
#define _BVGZ_BYTECODE_H

#include <stdio.h>
#include "parser.h"

// ASSUMES the 'parse' is correct & internally consistent.
int parse_to_bytecode(prs_result_t* parse, uint8_t** memory, uint32_t* size);
int bytecode_to_text(FILE* fp, void* memory, uint32_t* size);

#endif
