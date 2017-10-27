#include "tests.h"
#include "vm.h"


#define PRG_PATH    "/test/asm/sys/"


static void test_timers()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "timers.bvgzs");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(*((uint64_t*)vm->memory) == 1);

    destroy_vm(vm);
}


int main(int argc, char** argv)
{
    initialize_engine(NULL);

    test_timers();

    return 0;
}
