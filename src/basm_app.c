#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include "parser.h"
#include "bytecode.h"
#include "instns/instn.h"

#define DefaultEntryPointLabel  "_entry"


static list_t* include_paths = NULL;
static char* asmfile = NULL;
static char* outfile = NULL;
static char* entry_pt_label = NULL;
static int debug = 0;

static void print_help(FILE* out, char* program);
static void parse_args(int argc, char** argv);
static void free_args();


int main(int argc, char** argv)
{
    FILE* outfp = NULL;
    int retval = 0;
    uint32_t entry_offset = 0;

    setup_instn_defs();

    parse_args(argc, argv);

    prs_result_t* result = parse_asm(asmfile, include_paths);

    uint8_t* code = NULL;
    uint32_t codesz = 0;
    if (parse_to_bytecode(result, &code, &codesz, debug) < 0)
    {
        retval = 1;
        goto done;
    }

    if (resolve_data_label_refs(result) < 0)
    {
        retval = 1;
        goto done;
    }

    entry_offset = resolve_entry_point(entry_pt_label, result);
    if (entry_offset == (uint32_t)-1)
    {
        retval = 1;
        goto done;
    }

    outfp = fopen(outfile, "wb");
    if (!outfp)
    {
        fprintf(stderr, "open(%s): %s\n", outfile, strerror(errno));
        retval = 1;
        goto done;
    }

    if (write_bvgz_image(outfp, result, code, codesz,
        entry_offset, debug) < 0)
    {
        retval = 1;
        goto done;
    }

done:
    if (outfp) fclose(outfp);
    free(code);
    destroy_parse_result(result);
    free_args();
    return retval;
}


static void print_help(FILE* out, char* program)
{
    fprintf(out,
        "Usage %s [-g] [-p] [-I <include>] [-o <output>]"
        " [-e <label>] <file>\n",
        program);
}


static void make_output_filename(int asmfile_len)
{
    char buf[asmfile_len + 5];
    char* last_dot = strrchr(asmfile, '.');
    if (!last_dot)
    {
        *last_dot = 0;
    }

    sprintf(buf, "%s.bvg", asmfile);

    if (!last_dot)
    {
        *last_dot = '.';
    }

    outfile = strdup(buf);
}


static void print_instns()
{
    printf("Instructions: %u\n", nInstnDefs);
    for (uint32_t i = 0; i < nInstnDefs; i++)
    {
        instn_def_t* def = InstnDefs + i;
        printf(" [%2u] %8s /%d :: 0x%04x\n", i,
            def->name, def->arg_count, def->opcode);
    }
}


static void parse_args(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "hgpI:o:e:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(stdout, argv[0]);
                free_args();
                exit(0);
                break;

            case 'p':
                print_instns();
                free_args();
                exit(0);
                break;

            case 'g':
                debug = 1;
                break;

            case 'I':
                include_paths = list_prepend(include_paths,
                    list_make_node(strdup(optarg)));
                break;

            case 'o':
                outfile = strdup(optarg);
                break;

            case 'e':
                entry_pt_label = strdup(optarg);
                break;

            default:
                goto error;
        }
    }

    if (!entry_pt_label)
    {
        entry_pt_label = strdup(DefaultEntryPointLabel);
    }

    if (optind >= argc)
    {
        goto error;
    }

    asmfile = argv[optind];

    if (outfile == NULL)
    {
        make_output_filename(strlen(asmfile));
    }

    return;

error:
    print_help(stderr, argv[0]);
    free_args();
    exit(1);
}


static void free_args()
{
    for (list_t* n = include_paths; n; n = n->next)
    {
        free(n->data);
    }
    list_destroy(include_paths);

    free(entry_pt_label);
    free(outfile);
}
