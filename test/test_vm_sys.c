#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "tests.h"
#include "vm.h"


#define PRG_PATH    "/test/asm/sys/"


static void test_timers()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH "timers.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(*((uint64_t*)vm->memory) == 1);

    destroy_vm(vm);
}


static void test_file_open_close()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "file-open-close.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);
    assert(*((uint64_t*)(vm->memory + 8)) > 0); // the fd

    FILE* fp = fopen("Makefile", "rb");
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fclose(fp);

    // printf("sz = %lu / %ld\n", *((uint64_t*)(vm->memory + 16)), sz);

    assert(*((uint64_t*)(vm->memory + 16)) == sz); // the size

    destroy_vm(vm);
}


static void test_file_unlink()
{
    FILE* fp = fopen("test.file.123", "wb+");
    assert(fp != NULL);
    fclose(fp);

    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "file-unlink.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);
    assert(*((uint64_t*)(vm->memory + 8)) > 0); // the fd

    fp = fopen("test.file.123", "r");
    assert(fp == NULL);

    destroy_vm(vm);
}


static void test_file_stat()
{
    struct stat stat_s;
    int ret = stat("Makefile", &stat_s);
    assert(ret == 0);

    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "file-stat.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);

    ret = memcmp(&stat_s, vm->memory + 8, sizeof(struct stat));
    assert(ret == 0);

    destroy_vm(vm);
}


int main(int argc, char** argv)
{
    initialize_engine();

    test_timers();
    test_file_open_close();
    test_file_unlink();
    test_file_stat();

    return 0;
}
