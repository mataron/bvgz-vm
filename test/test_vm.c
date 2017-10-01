#include "tests.h"
#include "vm.h"
#include "instn.h"


#define PRG_PATH    "/test/asm/programs/"


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


static void test_cpn()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "cpn.s");

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


static void test_read()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "read.s");

    printf("read-mem-sz: %u\n", vm->memsz);
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


static void test_jmp()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "jmp.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(*(uint64_t*)(vm->memory) == 3);

    destroy_vm(vm);
}


static void test_ret()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "ret.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->instns == 1);

    destroy_vm(vm);
}


static void test_func()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "func.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(*(uint64_t*)(vm->memory) == 3);

    destroy_vm(vm);
}


static void test_fib_n(uint64_t n, uint64_t expect)
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "fib.s");

    *(uint64_t*)(vm->memory + 48) = n;

    execute_vm(vm);
    print_vm_state(vm);

    printf("fib(%lu) = %lu vs. %lu\n", n, *(uint64_t*)(vm->memory), expect);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(*(uint64_t*)(vm->memory) == expect);

    destroy_vm(vm);
}


static void test_fib()
{
    test_fib_n(0, 0);
    test_fib_n(1, 1);
    test_fib_n(2, 1);
    test_fib_n(3, 2);
    test_fib_n(4, 3);
    test_fib_n(5, 5);
    test_fib_n(6, 8);
    test_fib_n(7, 13);
    test_fib_n(8, 21);
    test_fib_n(9, 34);
    test_fib_n(10, 55);
    test_fib_n(11, 89);
    test_fib_n(12, 144);
    test_fib_n(13, 233);
    test_fib_n(14, 377);
}


int main(int argc, char** argv)
{
    setup_instn_defs();

    test_nop3();
    test_arithm();
    test_logical();
    test_bitwise();
    test_rel();
    test_cpn();
    test_read();
    test_cp();
    test_jmp();
    test_ret();
    test_func();
    test_fib();

    return 0;
}
