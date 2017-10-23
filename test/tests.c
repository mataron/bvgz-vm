#include "tests.h"
#include "instns/instn.h"
#include "parser.h"
#include "vm.h"
#include "bytecode.h"


vm_t* mk_vm_for_asm(char* asmfile)
{
    prs_result_t* parse = parse_asm(asmfile, NULL);
    assert(parse != NULL);
    assert(parse->errors == 0);

    uint8_t* code = NULL;
    uint32_t codesz = 0;
    int ret = parse_to_bytecode(parse, &code, &codesz, 1);
    assert(ret == 0);
    assert(codesz > 0);

    ret = resolve_data_label_refs(parse);
    assert(ret == 0);

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
