#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

#ifndef NDEBUG
#define DEBUG_SLEEP_BEFORE_EXEC 250000
#endif


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

    snprintf(buf, sizeof(buf), "%s/img-%04d%02d%02d-%02d%02d%02d-%s.bvgz",
        BVGZ_image_gen_dir,
        1900 + utc_tm.tm_year, 1 + utc_tm.tm_mon, utc_tm.tm_mday,
        utc_tm.tm_hour, utc_tm.tm_min, utc_tm.tm_sec,
        suffix
    );

    return strdup(buf);
}


static void save_child_proc(pid_t pid, vm_t* vm)
{
    if (vm->proc.n_proc >= vm->proc.alloc_proc)
    {
        vm->proc.child_proc = realloc(vm->proc.child_proc,
            sizeof(vm_child_t) * (vm->proc.alloc_proc + PROC_ALLOC));
        memset(vm->proc.child_proc + vm->proc.alloc_proc, 0,
            sizeof(vm_child_t) * PROC_ALLOC);
        vm->proc.alloc_proc += PROC_ALLOC;
    }

    for (uint32_t p = 0; p < vm->proc.alloc_proc; p++)
    {
        vm_child_t* ch = vm->proc.child_proc + p;
        if (ch->used) continue;

        ch->used = 1;
        ch->pid = pid;
        ch->exit_cb = NULL;
        ch->n_exit_cb = 0;

        vm->proc.n_proc++;
        return;
    }

    // unreachable!!!
    abort();
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

    // just ensure correctness here, the real argv array will be
    // built in the child process.
    for (uint64_t a = 0; a < argc; a++)
    {
        uint8_t* arg_ptr = deref_mem_ptr(argv_r[a], 1, vm);
        if (!arg_ptr || ensure_nul_term_str(arg_ptr, vm) < 0)
        {
            *ret = 0;
            return;
        }
    }

    pid_t ch_pid = fork();
    if (ch_pid == 0)
    {
        char** argv_cp = malloc(sizeof(char*) * (argc + 1));
        memset(argv_cp, 0, sizeof(char*) * (argc + 1));

        for (uint64_t a = 0; a < argc; a++)
        {
            uint8_t* arg_ptr = deref_mem_ptr(argv_r[a], 1, vm);
            argv_cp[a] = strdup((char*)arg_ptr);
        }

        destroy_vm(vm);

#ifndef NDEBUG
        usleep(DEBUG_SLEEP_BEFORE_EXEC);
#endif

        execv(argv_cp[0], argv_cp);
        fprintf(stderr, "execv(%s, argc=%lu) failed: %s\n",
            argv_cp[0], argc, strerror(errno));

        for (uint64_t a = 0; a < argc; a++)
        {
            free(argv_cp[a]);
        }
        free(argv_cp);
        exit(1);
        return;
    }

    if (ch_pid < 0)
    {
        *ret = 0;
        vm->error_no = errno;
    }
    else
    {
        *ret = ch_pid;
        save_child_proc(ch_pid, vm);
    }
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
        destroy_vm(vm); // make sure all i/o & misc resources are released

        char** argv = malloc(sizeof(char*) * 3);
        argv[0] = BVGZ_VM_executable;
        argv[1] = imgname;
        argv[2] = NULL;

#ifndef NDEBUG
        usleep(DEBUG_SLEEP_BEFORE_EXEC);
#endif
        execv(BVGZ_VM_executable, argv);
        fprintf(stderr, "execv(BVGZ VM) failed: %s\n", strerror(errno));

        free(imgname);
        free(argv);
        exit(1);
        return;
    }

    free(imgname);

    if (ch_pid < 0)
    {
        *ret = 0;
        vm->error_no = errno;
    }
    else
    {
        *ret = ch_pid;
        save_child_proc(ch_pid, vm);
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


static vm_child_t* find_proc(vm_t* vm, pid_t pid)
{
    for (uint32_t p = 0; p < vm->proc.alloc_proc; p++)
    {
        vm_child_t* child = vm->proc.child_proc + p;
        if (!child->used) continue;
        if (child->pid == pid)
        {
            return child;
        }
    }
    return NULL;
}


void sys_onexit(vm_t* vm, uint32_t argv, uint32_t retv)
{
    uint64_t* args = (uint64_t*)deref_mem_ptr(argv, 8, vm);
    uint64_t* ret = (uint64_t*)deref_mem_ptr(retv, 8, vm);
    if (!args || !ret)
    {
        vm->error_no = EFAULT;
        if (ret) *ret = 1;
        return;
    }

    if (!deref(args[1], 7 * 8, vm)) // cb_args
    {
        vm->error_no = EFAULT;
        *ret = 1;
        return;
    }

    if (args[2] >= vm->codesz) // callback
    {
        vm->error_no = EINVAL;
        *ret = 1;
        return;
    }

    vm_child_t* ch_ptr = find_proc(vm, args[0]);

    if (!ch_ptr)
    {
        vm->error_no = ECHILD;
        *ret = 1;
        return;
    }

    ch_ptr->exit_cb = realloc(ch_ptr->exit_cb,
        sizeof(vm_callback_t) * (ch_ptr->n_exit_cb + 1));
    vm_callback_t* cb = ch_ptr->exit_cb + ch_ptr->n_exit_cb;
    ch_ptr->n_exit_cb++;
    vm->proc.n_exit_callbacks++;

    cb->args = args[1];
    cb->callback = args[2];

    *ret = 0;
}


void destroy_vm_proc(vm_t* vm)
{
    for (uint32_t p = 0; p < vm->proc.alloc_proc; p++)
    {
        vm_child_t* ch = vm->proc.child_proc + p;
        if (!ch->used) continue;
        free(ch->exit_cb);
    }

    free(vm->proc.child_proc);
}


static uint32_t fire_events_of_proc(vm_t* vm, vm_child_t* ch_ptr,
    pid_t pid, int status)
{
    uint32_t n_events = 0;
    for (uint32_t c = 0; c < ch_ptr->n_exit_cb; c++)
    {
        vm_callback_t* cb = ch_ptr->exit_cb + c;
        uint64_t* cb_args =
            (uint64_t*)deref_mem_ptr(cb->args, 7 * 8, vm);
        if (!cb_args)
        {
            continue; // there isn't anything we can do...
        }

        cb_args[0] = pid;
        cb_args[1] = WIFEXITED(status);
        cb_args[2] = WEXITSTATUS(status);
        cb_args[3] = WIFSIGNALED(status);
        cb_args[4] = WTERMSIG(status);
        cb_args[5] = WIFSTOPPED(status);
        cb_args[6] = WIFCONTINUED(status);

        make_func_procedure(cb->callback, cb->args, 0, vm);
        n_events++;
    }
    return n_events;
}


uint32_t fire_proc_events(vm_t* vm)
{
    uint32_t n_events = 0;
    // the process is repeated until no processes are
    // left to report for or an error occurs
    while (vm->proc.n_proc)
    {
        int status = 0;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid < 0)
        {
            vm->error_no = errno;
            break;
        }
        if (pid == 0) break;

        vm_child_t* ch_ptr = find_proc(vm, pid);

        if (!ch_ptr) break;

        n_events += fire_events_of_proc(vm, ch_ptr, pid, status);

        if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            if (ch_ptr->n_exit_cb)
            {
                free(ch_ptr->exit_cb);
                vm->proc.n_exit_callbacks -= ch_ptr->n_exit_cb;
                ch_ptr->exit_cb = NULL;
                ch_ptr->n_exit_cb = 0;
            }
            ch_ptr->used = 0;
            vm->proc.n_proc--;
        }
    }

    return n_events;
}


int has_pending_proc_events(vm_t* vm)
{
    return vm->proc.n_exit_callbacks > 0;
}


void cleanup_child_proc(vm_t* vm)
{
    fire_proc_events(vm);
}
