#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "parser.h"
#include "bytecode.h"

#define DefaultEntryPointLabel  "_entry"


list_t* include_paths = NULL;
char* asmfile = NULL;
char* outfile = NULL;
char* entry_pt_label = NULL;


static void print_help(FILE* out, char* program);
static void parse_args(int argc, char** argv);
static void free_args();
static uint32_t resolve_entry_point(prs_result_t* result);


int main(int argc, char** argv)
{
    FILE* outfp = NULL;
    int retval = 0;
    uint32_t entry_offset = 0;

    parse_args(argc, argv);

    prs_result_t* result = parse_asm(asmfile, include_paths);

    uint8_t* code = NULL;
    uint32_t codesz = 0;
    if (parse_to_bytecode(result, &code, &codesz) < 0)
    {
        retval = 1;
        goto done;
    }

    entry_offset = resolve_entry_point(result);
    if (entry_offset == (uint32_t)-1)
    {
        retval = 1;
        goto done;
    }

    outfp = fopen(outfile, "wb");
    if (write_bvgz_image(outfp, result, code, codesz,
        entry_offset) < 0)
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
        "Usage %s [-I <include>] [-o <output>] [-e <label>] <file>\n",
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


static void parse_args(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "hI:o:e:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(stdout, argv[0]);
                free_args();
                exit(0);
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
        print_help(stderr, argv[0]);
        exit(1);
    }

    asmfile = argv[optind];

    make_output_filename(strlen(asmfile));
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


static uint32_t resolve_entry_point(prs_result_t* result)
{
    label_t* elbl = NULL;

    if (hmap_get(result->labels, entry_pt_label, (void**)&elbl) != MAP_OK)
    {
        fprintf(stderr, "entry label not found: %s\n",
            entry_pt_label);
        return (uint32_t)-1;
    }

    if (elbl->is_mem_ref)
    {
        fprintf(stderr, "entry label refers to memory segment: %s\n",
            entry_pt_label);
        return (uint32_t)-1;
    }

    return elbl->offset;
}