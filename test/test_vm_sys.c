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


static void test_file_seek()
{
    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "file-read-tail.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);
    assert(*((uint64_t*)(vm->memory + 8)) > 0); // the fd
    assert(*((uint64_t*)(vm->memory + 16)) == 768); // the size

    destroy_vm(vm);
}


static void test_dir_mkrm()
{
    struct stat stat_s;
    int ret = stat("test.dir.123", &stat_s);
    if (!ret)
    {
        ret = rmdir("test.dir.123");
        assert(ret == 0);
    }

    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "mkdir.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);

    destroy_vm(vm);

    ret = stat("test.dir.123", &stat_s);
    assert(ret == 0);
    assert(S_ISDIR(stat_s.st_mode));

    vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "rmdir.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);

    destroy_vm(vm);

    ret = stat("test.dir.123", &stat_s);
    assert(ret < 0);
}


static void test_readdir()
{
    struct stat stat_s;
    int ret = stat("test.dir.456", &stat_s);
    if (ret < 0)
    {
        ret = mkdir("test.dir.456",
            S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        assert(ret == 0);

        fclose(fopen("test.dir.456/a.txt", "wb+"));
        fclose(fopen("test.dir.456/b.txt", "wb+"));
        fclose(fopen("test.dir.456/c.txt", "wb+"));
    }

    vm_t* vm = mk_vm_for_asm(xstr(PROJECT_ROOT) PRG_PATH
        "readdir.s");

    execute_vm(vm);
    print_vm_state(vm);

    assert(vm->exceptions == 0);
    assert(vm->procedures == NULL);
    assert(vm->error_no == 0);
    assert(*((uint64_t*)vm->memory) == 0);
    // NOTE: the dir contents also contain directories '.' & '..'
    assert(*((uint64_t*)(vm->memory + 8)) == 5); // saved
    assert(*((uint64_t*)(vm->memory + 16)) == 5); // total
    assert(*((uint64_t*)(vm->memory + 24)) == 6 * 4 + 3 * 6 + 2 + 3); // bytes required

    destroy_vm(vm);

    unlink("test.dir.456/a.txt");
    unlink("test.dir.456/b.txt");
    unlink("test.dir.456/c.txt");
    rmdir("test.dir.456");
}


int main(int argc, char** argv)
{
    initialize_engine();

    test_timers();
    test_file_open_close();
    test_file_unlink();
    test_file_stat();
    test_file_seek();
    test_dir_mkrm();
    test_readdir();

    return 0;
}
