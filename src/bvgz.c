#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include "vm.h"
#include "bytecode.h"
#include "instn.h"


char* imgfile = NULL;
char* dump_file = NULL;
vm_t* vm = NULL;


static void print_help(FILE* out, char* program);
static void parse_args(int argc, char** argv);
static void cleanup();


int main(int argc, char** argv)
{
    int retval = 0;
    parse_args(argc, argv);

    setup_instn_defs();

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

    fclose(imgfp);
    imgfp = NULL;

    execute_vm(vm);
    print_vm_state(vm);

    destroy_vm(vm);

done:
    if (imgfp) fclose(imgfp);
    cleanup();
    return retval;
}


static void print_help(FILE* out, char* program)
{
    fprintf(out,
        "Usage %s [-v]+ [-hqs] [-D <dumb-file>]  <file>\n",
        program);
}


static void parse_args(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "hvVqsD:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(stdout, argv[0]);
                cleanup();
                exit(0);
                break;

            case 'D':
                dump_file = strdup(optarg);
                break;

            case 'v':
                verbose++;
                break;

            case 'q':
                verbose = -1;
                break;

            case 's':
                collect_stats = 1;
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
    free(dump_file);
    if (vm)
    {
        destroy_vm(vm);
    }
}
