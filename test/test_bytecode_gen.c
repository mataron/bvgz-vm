#include "tests.h"
#include "bytecode.h"
#include "parser.h"
#include "instns/instn.h"


static void test_empty()
{
    prs_result_t* result = parse_asm(
        xstr(PROJECT_ROOT) "/test/asm/empty.s", NULL);

    uint8_t* memory = NULL;
    uint32_t size = 0;
    parse_to_bytecode(result, &memory, &size, 1);

    assert(memory == NULL);
    assert(size == 0);

    destroy_parse_result(result);
}


static void test_gen()
{
    prs_result_t* result = parse_asm(
        xstr(PROJECT_ROOT) "/test/asm/bytecode-gen-test.s", NULL);

    uint8_t* memory = NULL;
    uint32_t size = 0;
    parse_to_bytecode(result, &memory, &size, 1);

    assert(size == 3 + 4 + 1 + 2);
    assert(memory != NULL);

    assert((*(uint16_t*)memory & 0x7) == 0x6);
    assert(memory[2] == 1 << 4);
    assert(*(uint32_t*)(memory + 3) == 0x1234);
    assert(memory[7] == 0x12);
    assert(*(uint16_t*)(memory + 8) == 0x123);

    free(memory);

    destroy_parse_result(result);
}


int main(int argc, char** argv)
{
    setup_instn_defs();

    test_empty();
    test_gen();

    return 0;
}
