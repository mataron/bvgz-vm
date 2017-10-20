#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"
#include "instns/instn.h"


int dbg_quit(int argc, char** argv, dbg_state_t* state)
{
    printf("Exiting...\n");
    return -1;
}


int dbg_help(int argc, char** argv, dbg_state_t* state)
{
    if (!state->help_format_string)
    {
        int mxlen = 0;
        for (dbg_command_t* c = Commands; c->name; c++)
        {
            int len = strlen(c->name);
            if (len > mxlen) mxlen = len;
        }

        state->help_format_string = malloc(20);
        snprintf(state->help_format_string, 19, "%%%ds\t%%s\n", mxlen + 1);
    }

    for (dbg_command_t* c = Commands; c->name; c++)
    {
        printf(state->help_format_string, c->name, c->description);
    }

    return 0;
}


int dbg_show_symbols(int argc, char** argv, dbg_state_t* state)
{
    if (argc == 1 || argv[1][0] != 'c')
    {
        printf("Memory symbols (%u):\n", state->data->n_mem_lines);
        for (uint32_t m = 0; m < state->data->n_mem_lines; m++)
        {
            dbg_line_assoc_t* mem = DBG_MEM_LINE(state->data, m);
            printf("  0x%08x:  %s\n", mem->address,
                DBG_LABEL(state->data, mem->label_ref));
        }
    }
    if (argc == 1 || argv[1][0] != 'm')
    {
        uint32_t n_code_labels = 0;
        for (uint32_t c = 0; c < state->data->n_code_lines; c++)
        {
            if (DBG_CODE_LINE(state->data, c)->label_ref >= 0)
            {
                n_code_labels++;
            }
        }

        printf("Code symbols (%u):\n", n_code_labels);
        for (uint32_t c = 0; c < state->data->n_code_lines; c++)
        {
            dbg_line_assoc_t* code = DBG_CODE_LINE(state->data, c);
            if (code->label_ref >= 0)
            {
                printf("  0x%08x:  %s\n", code->address,
                    DBG_LABEL(state->data, code->label_ref));
            }
        }
    }
    return 0;
}


int dbg_show_vm(int argc, char** argv, dbg_state_t* state)
{
    if (!state->flags) printf("VM Stopped\n");
    else if (state->flags & F_RUNNING) printf("VM Running\n");

    printf("Code sz: %5u | Mem sz: %u\n",
        state->vm->codesz, state->vm->memsz);

    printf("Instns run: %4lu | Since last cleanup: %u\n",
        state->vm->instns, state->vm->instns_since_last_cleanup);
    printf("Events: Tmrs:%3u | I/O fds:%2u | I/O events:%2u"
        " | Procs:%2u | Procs C/Bs:%2u\n", state->vm->n_timers,
        state->vm->io.used_fds, state->vm->io.n_io_events,
        state->vm->proc.n_proc, state->vm->proc.n_exit_callbacks);
    printf("Last error: %s\n", strerror(state->vm->error_no));

    if (state->vm->exceptions)
    {
        printf("Exceptions:\n");
        print_exception("  ", state->vm->exceptions);
    }

    printf("Runnable procedures: %u\n", list_size(state->vm->procedures));
    printf("Next instn: 0x%08x\n", state->vm->iptr);

    return 0;
}


static uint32_t parse_uint(char* n, int radix)
{
    char* end = NULL;
    uint32_t value = strtoul(n, &end, radix);
    if (*end) // require the full string to be valid.
    {
        return (uint32_t)-1;
    }

    return value;
}


static uint32_t resolve_mem_address(char* address_or_label,
    dbg_state_t* state)
{
    dbg_label_t* label = NULL;
    int ret = hmap_get(state->labels, address_or_label, (void**)&label);
    if (ret == MAP_OK)
    {
        if (!label->is_mem_ref)
        {
            return (uint32_t)-1;
        }

        return label->address;
    }

    if (address_or_label[0] == '0' && address_or_label[1] == 'x')
    {
        return parse_uint(address_or_label, 16);
    }

    return parse_uint(address_or_label, 10);
}


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


dbg_break_pt_t* create_and_attach_breakpoint(dbg_state_t* state,
    unsigned type)
{
    dbg_break_pt_t* bpt = malloc(sizeof(dbg_break_pt_t));

    bpt->type = type;
    bpt->brk_id = state->brk_id_pool++;

    list_t* node = list_make_node(bpt);
    list_prepend(state->breakpoints, node);
    state->breakpoints = node;

    return bpt;
}


int dbg_break(int argc, char** argv, dbg_state_t* state)
{
    if (argc != 2)
    {
        printf("Usage: %s [label|address|:line|file:line|+instns|instn]\n",
            argv[0]);
        return 0;
    }


    if (argv[1][0] == '+')
    {
        uint32_t count = parse_uint(argv[1] + 1, 10);
        if (count == (uint32_t)-1)
        {
            printf("invalid instruction count: [%s]\n", argv[1] + 1);
            return 0;
        }

        dbg_break_pt_t* bpt =
            create_and_attach_breakpoint(state, BRK_T_InstnCount);
        bpt->point.instn_count = state->vm->instns + count;
        return 0;
    }

    char* colon = strchr(argv[1], ':');
    if (colon)
    {
        *colon = 0;
        char* file = argv[1];
        uint32_t lineno = parse_uint(colon + 1, 10);
        if (lineno == (uint32_t)-1)
        {
            printf("bad line number: [%s]\n", colon + 1);
            return 0;
        }

        for (uint32_t c = 0; c < state->data->n_code_lines; c++)
        {
            dbg_line_assoc_t* code = DBG_CODE_LINE(state->data, c);
            char* c_file = DBG_FILE(state->data, code->fileno);
            if ((!*file || !strcmp(file, c_file)) && lineno == code->lineno)
            {
                dbg_break_pt_t* bpt =
                    create_and_attach_breakpoint(state, BRK_T_Line);
                bpt->point.line.address = code->address;
                bpt->point.line.file = c_file;
                bpt->point.line.lineno = lineno;
                return 0;
            }
        }
    }

    dbg_label_t* label = NULL;
    int ret = hmap_get(state->labels, argv[1], (void**)&label);
    if (ret == MAP_OK)
    {
        if (label->is_mem_ref)
        {
            printf("label [%s] points into data segment\n", argv[1]);
            return 0;
        }

        dbg_break_pt_t* bpt =
            create_and_attach_breakpoint(state, BRK_T_Label);
        bpt->point.label.name = label->label;
        bpt->point.label.address = label->address;
        return 0;
    }

    instn_def_t* fmt = bsearch(&argv[1], InstnDefs, nInstnDefs,
        sizeof(instn_def_t), compare_instn_def);
    if (fmt)
    {
        dbg_break_pt_t* bpt =
            create_and_attach_breakpoint(state, BRK_T_InstnName);
        bpt->point.instn_name = (char*)fmt->name;
        return 0;
    }

    if (argv[1][0] == '0' && argv[1][1] == 'x')
    {
        uint32_t address = parse_uint(argv[1], 16);
        if (address != (uint32_t)-1)
        {
            dbg_break_pt_t* bpt =
                create_and_attach_breakpoint(state, BRK_T_Address);
            bpt->point.address = address;
            return 0;
        }
    }

    uint32_t address = parse_uint(argv[1], 10);
    if (address != (uint32_t)-1)
    {
        dbg_break_pt_t* bpt =
            create_and_attach_breakpoint(state, BRK_T_Address);
        bpt->point.address = address;
        return 0;
    }

    printf("cannot parse breakpoint: [%s]\n", argv[1]);
    return 0;
}


static const char BrkTypes[] = { 'A', 'L', 'F', 'I', '+' };

int dbg_show_breakpts(int argc, char** argv, dbg_state_t* state)
{
    uint32_t sz = list_size(state->breakpoints);
    printf("Breakpoints (%u):\n", sz);

    for (list_t* n = state->breakpoints; n; n = n->next)
    {
        dbg_break_pt_t* brk = n->data;
        printf("[%3u] %c: ", brk->brk_id, BrkTypes[brk->type - 1]);
        switch (brk->type)
        {
        case BRK_T_Address:
            printf("0x%08x", brk->point.address);
            break;
        case BRK_T_Label:
            printf("0x%08x :: %s", brk->point.label.address,
                brk->point.label.name);
            break;
        case BRK_T_Line:
            printf("0x%08x :: %s:%u", brk->point.line.address,
                brk->point.line.file, brk->point.line.lineno);
            break;
        case BRK_T_InstnName:
            printf("[%s]", brk->point.instn_name);
            break;
        case BRK_T_InstnCount:
            printf("[%u]", brk->point.instn_count);
            break;
        }
        printf("\n");
    }

    return 0;
}


int dbg_break_delete(int argc, char** argv, dbg_state_t* state)
{
    if (argc != 2)
    {
        printf("Usage: %s [brk id]\n",
            argv[0]);
        return 0;
    }

    uint32_t brkid = parse_uint(argv[1], 10);
    if (brkid == (uint32_t)-1)
    {
        printf("invalid breakpoint id: [%s]\n", argv[1]);
        return 0;
    }

    for (list_t* n = state->breakpoints; n; n = n->next)
    {
        dbg_break_pt_t* brk = n->data;
        if (brk->brk_id == brkid)
        {
            if (state->breakpoints == n)
            {
                state->breakpoints = n->next;
            }
            list_unlink(n);
            free(brk);
            free(n);
            return 0;
        }
    }

    printf("breakpoint not found: [%u]\n", brkid);
    return 0;
}


dbg_command_t Commands[] = {
    { "q", "quit the debugger", dbg_quit },
    { "help", "show this message", dbg_help },
    { "sym", "show symbols: sym [c|m]", dbg_show_symbols },
    { "vm", "show vm state", dbg_show_vm },
    { "db", "dump byte", dbg_dump_byte },
    { "dw", "dump word", dbg_dump_word },
    { "dd", "dump doubleword", dbg_dump_dword },
    { "dq", "dump quadword", dbg_dump_qword },
    { "ds", "dump string", dbg_dump_str },
    { "brk", "set breakpoint", dbg_break },
    { "brks", "show breakpoints", dbg_show_breakpts },
    { "brkd", "delete breakpoint", dbg_break_delete },
    { NULL, NULL, NULL }
};
