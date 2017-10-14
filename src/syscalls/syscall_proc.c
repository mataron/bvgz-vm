#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include "vm.h"
#include "syscall_proc.h"
#include "bytecode.h"


#define xstr(s) str(s)
#define str(s) #s

char* BVGZ_image_gen_dir = xstr(BVGZ_IMG_GEN_DIR);


static void setup_image_gen_dir()
{
    struct stat stat_s;
    if (stat(BVGZ_image_gen_dir, &stat_s) < 0)
    {
        if (mkdir(BVGZ_image_gen_dir, S_IRWXU | S_IRWXG | S_IRWXO) < 0)
        {
            fprintf(stderr, "FATAL: mkdir(%s) failed\n", BVGZ_image_gen_dir);
            exit(1);
            return;
        }
    }
}

#define MAX_IMG_FILE_NAME_LEN   4096
#define IMG_FILE_NAME_SFX_LEN   32
char HexChars[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F'
};

static char* make_image_file_name()
{
    char buf[4096];
    char suffix[IMG_FILE_NAME_SFX_LEN + 1];

    time_t now = time(NULL);
    struct tm utc_tm;
    gmtime_r(&now, &utc_tm);

    for (unsigned i = 0; i < IMG_FILE_NAME_SFX_LEN; ++i)
    {
        suffix[i] = HexChars[rand() % 16];
    }
    suffix[IMG_FILE_NAME_SFX_LEN] = 0;

    snprintf(buf, sizeof(buf), "%s/img-%4d%2d%2d-%2d%2d%2d-%s.bvgz",
        BVGZ_image_gen_dir,
        1900 + utc_tm.tm_year, 1 + utc_tm.tm_mon, utc_tm.tm_mday,
        utc_tm.tm_hour, utc_tm.tm_min, utc_tm.tm_sec,
        suffix
    );

    return strdup(buf);
}


void sys_exec(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint8_t* args = deref_mem_ptr(argv, 8 + 4, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 0;
        return;
    }

    uint64_t argc = *(uint64_t*)args;
    if (!argc)
    {
        vm->error_no = EINVAL;
        *ret = 0;
        return;
    }

    uint32_t argv_ref = *(uint32_t*)(args + 8);

    uint32_t* argv_r = (uint32_t*)deref_mem_ptr(argv_ref, argc * 4, vm);
    if (!argv_r)
    {
        *ret = 0;
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
        return;
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
    uint8_t* args = deref_mem_ptr(argv, 2 * 4 + 3 * 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 0;
        return;
    }

    uint32_t code_ref = *(uint32_t*)args;
    uint64_t codesz = *(uint64_t*)(args + 4);
    uint32_t mem_ref = *(uint32_t*)(args + 12);
    uint64_t memsz = *(uint64_t*)(args + 16);
    uint64_t entry_pt = *(uint64_t*)(args + 24);

    if (entry_pt >= codesz || !FITS_IN_32Bit(codesz) ||
        !FITS_IN_32Bit(memsz))
    {
        vm->error_no = EINVAL;
        *ret = 0;
        return;
    }

    uint8_t* code_ptr = deref_mem_ptr(code_ref, codesz, vm);
    if (!code_ptr)
    {
        *ret = 0;
        return;
    }

    uint8_t* mem_ptr = deref_mem_ptr(mem_ref, memsz, vm);
    if (!mem_ptr)
    {
        *ret = 0;
        return;
    }

    setup_image_gen_dir();
    char* imgname = make_image_file_name();
    FILE *imgfp = fopen(imgname, "wb");
    if (!imgfp)
    {
        free(imgname);
        vm->error_no = errno;
        *ret = 0;
        return;
    }

    int wret = write_bvgz_image_direct(imgfp, code_ptr, codesz,
        mem_ptr, memsz, entry_pt);

    fclose(imgfp);
    if (wret < 0)
    {
        free(imgname);
        vm->error_no = errno;
        *ret = 0;
        return;
    }

    pid_t ch_pid = fork();
    if (ch_pid == 0)
    {
        char** argv = malloc(sizeof(char*) * 3);
        argv[0] = BVGZ_VM_executable;
        argv[1] = imgname;
        argv[2] = NULL;

        execv(BVGZ_VM_executable, argv);
        fprintf(stderr, "execv(BVGZ VM) failed: %s\n", strerror(errno));
        exit(1);
        return;
    }

    free(imgname);

    if (ch_pid < 0)
    {
        *ret = 0;
        vm->error_no = errno;
    }
    else if (ch_pid > 0)
    {
        *ret = ch_pid;
    }
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
