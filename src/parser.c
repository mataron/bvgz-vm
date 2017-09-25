#define _GNU_SOURCE // for getline()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "parser.h"

#define FILEPATH_BUF_SZ 256
#define MAX_TOKENS  4   // maximum number of tokens per line

#define P_ERROR 0
#define P_WARN  1

static char* ReportKinds[] = {
    "Error",
    "Warning"
};

static void report(int kind, char* source, uint32_t line, char* message, ...);

static prs_result_t* init_parse_result();
static void destroy_node(prs_node_t*);

static int find_file(char* filename, list_t* include_paths,
    char* filepath, int filepath_sz);
static void parse_asm_into(char* filename, list_t* include_paths,
    prs_result_t* result);

static void parse_line(char* filename, uint32_t lineno, char* line,
    list_t* include_paths, prs_result_t* result);

static void add_label(char* label_token, prs_result_t* result);
static void add_instn(char* filename, uint32_t lineno, char** tokens,
    prs_result_t* result);
static void add_node(prs_node_t* node, prs_result_t* result);


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
    for (uint32_t i = 0; i < parse_result->n_nodes; ++i)
    {
        destroy_node(parse_result->nodes[i]);
    }
    free(parse_result->nodes);

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
    res->n_nodes = 0;
    res->nodes = NULL;
    return res;
}


static void destroy_node(prs_node_t* instn)
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
    uint32_t lineno = 0;
    while (getline(&line, &sz, fp) > 0)
    {
        lineno++;
        parse_line(filename, lineno, line, include_paths, result);
        free(line);
    }

    fclose(fp);
}


static void parse_line(char* filename, uint32_t lineno, char* line,
    list_t* include_paths, prs_result_t* result)
{
    char* tokens[MAX_TOKENS];
    for (int i = 0; i < MAX_TOKENS; i++)
    {
        tokens[i] = NULL;
    }

    char* p = line;
    while (*p && isspace(*p)) p++;

    // empty line or comment
    if (!*p || *p == ';') return;

    tokens[0] = p;

    int parse_instn = 1;
    for (int tkn = 0, mark_token = 0; *p; p++)
    {
        if (*p == ';')
        {
            *p = 0;
            break;
        }
        else if (isspace(*p))
        {
            *p = 0;
            mark_token = 1;
        }
        else if (tkn == 0 && *p == ':')
        {
            *p = 0;
            add_label(tokens[0], result);
            parse_instn = 0;
        }
        else if (!parse_instn)
        {
            report(P_ERROR, filename, lineno,
                "end of line expected, found: '%c'", *p);
            return;
        }
        else if (mark_token)
        {
            tkn++;
            tokens[tkn] = p;
            mark_token = 0;
        }
    }

    if (parse_instn)
    {
        add_instn(filename, lineno, tokens, result);
    }
}


static void add_label(char* label_token, prs_result_t* result)
{
    prs_label_t* label = malloc(sizeof(prs_label_t));
    label->type = PT_LABEL;
    label->label = strdup(label_token);
    add_node((prs_node_t*)label, result);
}


static void add_instn(char* filename, uint32_t lineno, char** tokens,
    prs_result_t* result)
{

}


static void add_node(prs_node_t* node, prs_result_t* result)
{
    result->n_nodes++;
    result->nodes = realloc(result->nodes,
        result->n_nodes * sizeof(prs_node_t*) );

    result->nodes[result->n_nodes - 1] = node;
}
