#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "debug_util.h"
#include "instns/instn.h"


static uint32_t dump_cmd_prefix(int argc, char** argv, dbg_state_t* state,
    int size)
{
    if (argc != 2)
    {
        printf("Usage: %s [label|address]\n", argv[0]);
        return (uint32_t)-1;
    }

    uint32_t address = resolve_mem_address(argv[1], state);
    if (address == (uint32_t)-1)
    {
        printf("bad mem address: %s\n", argv[1]);
        return (uint32_t)-1;
    }

    printf("0x%08x: ", address);

    if (address + size > state->vm->memsz)
    {
        printf("out of range\n");
        return (uint32_t)-1;
    }

    return address;
}


int dbg_dump_byte(int argc, char** argv, dbg_state_t* state)
{
    uint32_t address = dump_cmd_prefix(argc, argv, state, 1);
    if (address == (uint32_t)-1)
    {
        return 0;
    }

    int v = *(uint8_t*)(state->vm->memory + address);
    printf("0x%02x (%3d)\n", v, v);

    return 0;
}


int dbg_dump_word(int argc, char** argv, dbg_state_t* state)
{
    uint32_t address = dump_cmd_prefix(argc, argv, state, 2);
    if (address == (uint32_t)-1)
    {
        return 0;
    }

    int v = *(uint16_t*)(state->vm->memory + address);
    printf("0x%04x (%5d)\n", v, v);

    return 0;
}


int dbg_dump_dword(int argc, char** argv, dbg_state_t* state)
{
    uint32_t address = dump_cmd_prefix(argc, argv, state, 4);
    if (address == (uint32_t)-1)
    {
        return 0;
    }

    uint32_t v = *(uint32_t*)(state->vm->memory + address);
    printf("0x%08x (%u)\n", v, v);

    return 0;
}


int dbg_dump_qword(int argc, char** argv, dbg_state_t* state)
{
    uint32_t address = dump_cmd_prefix(argc, argv, state, 8);
    if (address == (uint32_t)-1)
    {
        return 0;
    }

    uint64_t v = *(uint64_t*)(state->vm->memory + address);
    printf("0x%016lx (%lu)\n", v, v);

    return 0;
}


int dbg_dump_str(int argc, char** argv, dbg_state_t* state)
{
    if (argc != 2)
    {
        printf("Usage: %s [label|address]\n", argv[0]);
        return 0;
    }

    uint32_t address = resolve_mem_address(argv[1], state);
    if (address == (uint32_t)-1)
    {
        printf("bad mem address: %s\n", argv[1]);
        return 0;
    }

    printf("0x%08x: ", address);

    uint32_t len = 0;
    int terminated = 0;
    for (uint32_t i = address; i < state->vm->memsz; i++, len++)
    {
        if (!*(char*)(state->vm->memory + i))
        {
            terminated = 1;
            break;
        }
    }

    if (!terminated)
    {
        printf("string not terminated\n");
        return 0;
    }

    printf("len=%u [%s]\n", len, (char*)(state->vm->memory + address));
    return 0;
}
