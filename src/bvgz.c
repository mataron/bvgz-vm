#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include "vm.h"
#include "bytecode.h"
#include "instns/instn.h"
#include "syscalls/syscall.h"


static char* imgfile = NULL;
static int custom_temp_dir = 0;
static vm_t* vm = NULL;
static int debug = 0;


static void print_help(FILE* out, char* program);
static void parse_args(int argc, char** argv);
static void cleanup();


int main(int argc, char** argv)
{
    int retval = 0;
    parse_args(argc, argv);

    initialize_engine(argv[0]);

    FILE* imgfp = fopen(imgfile, "rb");
    if (!imgfp)
    {
        fprintf(stderr, "open(%s): %s\n", imgfile, strerror(errno));
        retval = 1;
        goto done;
    }

    vm = read_bvgz_image(imgfp);
    if (!vm)
    {
        retval = 1;
        goto done;
    }

    vm_debug_data_t* dbg = NULL;
    if (debug)
    {
        dbg = read_bvgz_debug_data(imgfp);
    }

    fclose(imgfp);
    imgfp = NULL;

    if (debug)
    {
        debug_vm(vm, dbg);
    }
    else
    {
        execute_vm(vm);
        print_vm_state(vm);
    }

    retval = vm->exceptions << 1;

done:
    free(dbg);
    if (imgfp) fclose(imgfp);
    cleanup();
    return retval;
}


static void print_help(FILE* out, char* program)
{
    fprintf(out,
        "Usage %s [-hd] [-G <temp-dir>] <file>\n",
        program);
}


static void parse_args(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "hdG:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(stdout, argv[0]);
                cleanup();
                exit(0);
                break;

            case 'd':
                debug = 1;
                break;

            case 'G':
                BVGZ_image_gen_dir = strdup(optarg);
                custom_temp_dir = 1;
                break;

            default:
                goto error;
        }
    }

    if (optind >= argc)
    {
        goto error;
    }

    imgfile = argv[optind];
    return;

error:
    print_help(stderr, argv[0]);
    cleanup();
    exit(1);
}


static void cleanup()
{
    if (custom_temp_dir)
    {
        free(BVGZ_image_gen_dir);
    }
    if (vm)
    {
        destroy_vm(vm);
        vm = NULL;
    }
}
