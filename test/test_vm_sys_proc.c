#include "tests.h"
#include "vm.h"


#define PRG_PATH    "/test/asm/sys/"


static void test_exec_ls()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "exec-ls.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(*((uint64_t*)vm->memory) == 1);

    destroy_vm(vm);
}


static void test_exec_vm()
{
    vm_t* test_vm = mk_vm_for_asm(
        xstr(PROJECT_ROOT) "/test/asm/programs/nop3.s");

    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "exec-vm.s");

    *(uint64_t*)(vm->memory + 136) = test_vm->codesz;
    *(uint64_t*)(vm->memory + 144) = test_vm->memsz;
    *(uint64_t*)(vm->memory + 152) = ((proc_t*)test_vm->procedures)->iptr;

    uint32_t extra_mem = test_vm->codesz + test_vm->memsz - 1;
    vm->memsz += extra_mem;
    vm->memory = realloc(vm->memory, vm->memsz);
    memcpy(vm->memory + 160, test_vm->code, test_vm->codesz);
    memcpy(vm->memory + 160 + test_vm->codesz, test_vm->memory,
        test_vm->memsz);

    destroy_vm(test_vm);

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(*((uint64_t*)vm->memory) == 1);

    destroy_vm(vm);
}


int main(int argc, char** argv)
{
    initialize_engine("./src/bvgz");

    test_exec_ls();
    test_exec_vm();

    return 0;
}
