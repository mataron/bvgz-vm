#define _GNU_SOURCE // for getline()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "parser.h"

#define FILEPATH_BUF_SZ 256

#define P_ERROR 0
#define P_WARN  1

char* ReportKinds[] = {
    "Error",
    "Warning"
};

static void report(int kind, char* source, uint32_t line, char* message, ...);

static prs_result_t* init_parse_result();
static void destroy_instruction(prs_instn_t*);
static int find_file(char* filename, list_t* include_paths,
    char* filepath, int filepath_sz);
static void parse_asm_into(char* filename, list_t* include_paths,
    prs_result_t* result);
static void parse_line(char* line, list_t* include_paths,
    prs_result_t* result);


prs_result_t* parse_asm(char* filename, list_t* include_paths)
{
    char filepath[FILEPATH_BUF_SZ];
    if (find_file(filename, include_paths, filepath, FILEPATH_BUF_SZ) < 0)
    {
        report(P_ERROR, "<root>", 0, "file not found: %s", filename);
        return NULL;
    }

    prs_result_t* result = init_parse_result();

    parse_asm_into(filename, include_paths, result);

    return result;
}


void destroy_parse_result(prs_result_t* parse_result)
{
    for (uint32_t i = 0; i < parse_result->n_instructions; ++i)
    {
        destroy_instruction(parse_result->instructions[i]);
    }
    free(parse_result->instructions);

    free(parse_result);
}


static void report(int kind, char* source, uint32_t line, char* message, ...)
{
    va_list ap;

    fprintf(stderr, "%s [%s@%u]: ", ReportKinds[kind], source, line);

    va_start(ap, message);
    vfprintf(stderr, message, ap);
    va_end(ap);
}


static prs_result_t* init_parse_result()
{
    prs_result_t* res = malloc(sizeof(prs_result_t));
    res->n_instructions = 0;
    res->instructions = NULL;
    return res;
}


static void destroy_instruction(prs_instn_t* instn)
{
    // TODO: free instruction
    free(instn);
}


static int find_file(char* filename, list_t* include_paths,
    char* filepath, int filepath_sz)
{
    struct stat st;
    if (stat(filename, &st) == 0)
    {
        strncpy(filepath, filename, filepath_sz);
        return 0;
    }

    for (list_t* p = include_paths; p; p = p->next)
    {
        snprintf(filepath, filepath_sz, "%s/%s", (char*)p->data, filename);
        if (stat(filepath, &st) == 0) return 0;
    }

    return -1;
}


static void parse_asm_into(char* filename, list_t* include_paths,
    prs_result_t* result)
{
    FILE* fp = fopen(filename, "r");

    char* line = NULL;
    size_t sz = 0;
    while (getline(&line, &sz, fp) > 0)
    {
        parse_line(line, include_paths, result);
        free(line);
    }

    fclose(fp);
}


static void parse_line(char* line, list_t* include_paths,
    prs_result_t* result)
{
    char* p = line;
    while (*p && isspace(*p)) p++;

    // empty line or comment
    if (!*p || *p == ';') return;
}
