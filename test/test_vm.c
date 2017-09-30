#include "tests.h"
#include "instn.h"
#include "parser.h"
#include "vm.h"
#include "bytecode.h"


#define PRG_PATH    "/test/asm/programs/"


static vm_t* mk_vm_for_asm(char* asmfile)
{
    prs_result_t* parse = parse_asm(asmfile, NULL);
    assert(parse != NULL);

    uint8_t* code = NULL;
    uint32_t codesz = 0;
    int ret = parse_to_bytecode(parse, &code, &codesz);
    assert(ret == 0);
    assert(codesz > 0);

    uint32_t entry_offset = resolve_entry_point("_entry", parse);
    assert(entry_offset != (uint32_t)-1);

    vm_t* vm = make_vm(codesz, parse->memsz, entry_offset);
    assert(vm);

    memcpy(vm->code, code, codesz);
    memcpy(vm->memory, parse->memory, parse->memsz);

    destroy_parse_result(parse);
    free(code);
    return vm;
}


static void test_nop3()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "nop3.s");

    assert(vm->codesz == sizeof(uint16_t) * 3);
    assert(vm->memsz == 0);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 3);
    assert(vm->memsz == 0);

    destroy_vm(vm);
}


static void test_arithm()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "arithm.s");

    assert(vm->memsz == sizeof(uint64_t) * 5);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 5);
    assert(*(uint64_t*)(vm->memory) == 5 + 3);
    assert(*(uint64_t*)(vm->memory + 8) == 5 - 3);
    assert(*(uint64_t*)(vm->memory + 16) == 5 * 3);
    assert(*(uint64_t*)(vm->memory + 24) == 15 / 5);
    assert(*(uint64_t*)(vm->memory + 32) == 5 % 3);

    destroy_vm(vm);
}


static void test_logical()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "logical.s");

    assert(vm->memsz == sizeof(uint64_t) * 8);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 8);
    assert(*(uint64_t*)(vm->memory) == (0 && 1));
    assert(*(uint64_t*)(vm->memory + 8) == (1 && 1));
    assert(*(uint64_t*)(vm->memory + 16) == (0 || 1));
    assert(*(uint64_t*)(vm->memory + 24) == (0 || 0));
    assert(*(uint64_t*)(vm->memory + 32) == !1);
    assert(*(uint64_t*)(vm->memory + 40) == !0);
    assert(*(uint64_t*)(vm->memory + 48) == !!1);
    assert(*(uint64_t*)(vm->memory + 56) == !!0);

    destroy_vm(vm);
}


static void test_bitwise()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "bitwise.s");

    assert(vm->memsz == sizeof(uint64_t) * 6);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 6);
    assert(*(uint64_t*)(vm->memory) == (0x4));
    assert(*(uint64_t*)(vm->memory + 8) == 0x44);
    assert(*(uint64_t*)(vm->memory + 16) == 0x0f0f0f0f0f0f0f0fULL);
    assert(*(uint64_t*)(vm->memory + 24) == 0x40);
    assert(*(uint64_t*)(vm->memory + 32) == 0x40000);
    assert(*(uint64_t*)(vm->memory + 40) == 0x4);

    destroy_vm(vm);
}


static void test_rel()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "rel.s");

    assert(vm->memsz == sizeof(uint64_t) * 6);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 6);
    assert(*(uint64_t*)(vm->memory) == 0);
    assert(*(uint64_t*)(vm->memory + 8) == 1);
    assert(*(uint64_t*)(vm->memory + 16) == 1);
    assert(*(uint64_t*)(vm->memory + 24) == 0);
    assert(*(uint64_t*)(vm->memory + 32) == 1);
    assert(*(uint64_t*)(vm->memory + 40) == 1);

    destroy_vm(vm);
}


static void test_set()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "set.s");

    assert(vm->memsz == 30);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 8);
    assert(*(uint8_t*)(vm->memory) == 0x11);
    assert(*(uint16_t*)(vm->memory + 1) == 0x1122);
    assert(*(uint32_t*)(vm->memory + 3) == 0x11223344);
    assert(*(uint64_t*)(vm->memory + 7) == 0x1122334455667788);
    assert(*(uint8_t*)(vm->memory + 15) == 0x11);
    assert(*(uint16_t*)(vm->memory + 16) == 0x1122);
    assert(*(uint32_t*)(vm->memory + 18) == 0x11223344);
    assert(*(uint64_t*)(vm->memory + 22) == 0x1122334455667788);

    destroy_vm(vm);
}


static void test_deref()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "deref.s");

    printf("deref-mem-sz: %u\n", vm->memsz);
    assert(vm->memsz == 91);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 5);
    assert(*(uint8_t*)(vm->memory) == 0x88);
    assert(*(uint16_t*)(vm->memory + 1) == 0x7788);
    assert(*(uint32_t*)(vm->memory + 3) == 0x55667788);
    assert(*(uint64_t*)(vm->memory + 7) == 0x1122334455667788);

    destroy_vm(vm);
}


static void test_cp()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "cp.s");

    assert(vm->memsz == 126);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 3);
    assert(strncmp((char*)vm->memory, "this is a nice string", 22) == 0);

    destroy_vm(vm);
}


int main(int argc, char** argv)
{
    setup_instn_defs();

    test_nop3();
    test_arithm();
    test_logical();
    test_bitwise();
    test_rel();
    test_set();
    test_deref();
    test_cp();

    return 0;
}
