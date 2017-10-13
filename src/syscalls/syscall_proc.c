#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "vm.h"
#include "syscall.h"
#include "bytecode.h"


void sys_exec(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref_mem_ptr(argv, 8 + 4, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint64_t argc = *(uint64_t*)args;
    if (!argc)
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return;
    }

    uint32_t argv_ref = *(uint32_t*)(args + 8);

    uint32_t* argv_r = (uint32_t*)deref_mem_ptr(argv_ref, argc * 4, vm);
    if (!argv_r)
    {
        *ret = 1;
        return;
    }

    char** argv_cp = (char**)malloc(sizeof(char*) * (argc + 1));
    memset(argv_cp, 0, sizeof(char*) * (argc + 1));

    for (uint64_t a = 0; a < argc; a++)
    {
        uint8_t* arg_ptr = deref_mem_ptr(argv_r[a], 1, vm);
        if (ensure_nul_term_str(arg_ptr, vm) < 0)
        {
            goto free_argv_cp;
        }
        argv_cp[a] = strdup((char*)arg_ptr);
    }

    pid_t ch_pid = fork();
    if (ch_pid == 0)
    {
        execv(argv_cp[0], argv_cp);
        fprintf(stderr, "execv(%s, argc=%lu) failed: %s\n",
            argv_cp[0], argc, strerror(errno));
        exit(1);
    }

    if (ch_pid < 0)
    {
        *ret = 0;
        vm->error_no = errno;
    }
    else if (ch_pid > 0)
    {
        *ret = ch_pid;
    }

free_argv_cp:
    for (uint64_t a = 0; a < argc; a++)
    {
        free(argv_cp[a]);
    }
    free(argv_cp);
}


void sys_run(vm_t* vm, uint32_t argv, uint32_t retv)
{
}


void sys_kill(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args = (uint64_t*)deref_mem_ptr(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    uint64_t pid = args[0];

    int kret = kill(pid, SIGTERM);
    if (kret < 0)
    {
        vm->error_no = errno;
        *ret = 1;
        return;
    }

    *ret = 0;
}


void sys_onexit(vm_t* vm, uint32_t argv, uint32_t retv)
{
}
