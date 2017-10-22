#include "tests.h"
#include "bytecode.h"
#include "parser.h"
#include "instns/instn.h"
#include "vm.h"


static void test_decode()
{
    vm_t vm;
    vm.memsz = 0x1234 + 8;

    prs_result_t* result = parse_asm(
        xstr(PROJECT_ROOT) "/test/asm/bytecode-gen-test.s", NULL);

    uint8_t* memory = NULL;
    uint32_t size = 0;
    parse_to_bytecode(result, &memory, &size, 1);

    destroy_parse_result(result);

    vm.code = memory;
    vm.codesz = size;
    vm.memory = memory;
    instn_t instn;
    int32_t offset = decode_instn(memory, &vm, &instn);

    assert(offset != -1);
    assert(offset == size);

    assert(instn.args[0].ptr == memory + 0x1234);
    assert(instn.args[1].u8 == 0x12);
    assert(instn.args[2].u16 == 0x123);

    free(memory);
}


int main(int argc, char** argv)
{
    setup_instn_defs();

    test_decode();

    return 0;
}
