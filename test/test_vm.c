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


int main(int argc, char** argv)
{
    setup_instn_defs();

    test_nop3();
    test_arithm();

    return 0;
}
