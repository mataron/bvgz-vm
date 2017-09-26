#define _GNU_SOURCE // for getline()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

#include "parser.h"
#include "util/hashset.h"

// TODO: preprocessor directives (at least include)

#define FILEPATH_BUF_SZ 256
#define MAX_TOKENS  4   // maximum number of tokens per line

#define P_ERROR 0
#define P_WARN  1

static char* ReportKinds[] = {
    "Error",
    "Warning"
};

static void report(int kind, const char* source, uint32_t line, char* message, ...);

static prs_result_t* init_parse_result();
static void destroy_instn(prs_instn_t*);

static int find_file(const char* filename, list_t* include_paths,
    char* filepath, int filepath_sz);
static void parse_asm_into(const char* filename, list_t* include_paths,
    prs_result_t* result, hashset_t label_refs);

static void parse_line(const char* filename, uint32_t lineno, char* line,
    list_t* include_paths, prs_result_t* result, hashset_t label_refs);

static void add_label(const char* label_token, prs_result_t* result);
static void add_instn(const char* filename, uint32_t lineno, const char** tokens,
    int n_tokens, prs_result_t* result, hashset_t label_refs);

static void parse_instn(const char* filename, uint32_t lineno,
	const char** tokens, int n_tokens, prs_instn_t** result, hashset_t label_refs);
static int parse_arg(const char* filename, uint32_t lineno,
	const char* token, int arg_index, prs_arg_t* result, hashset_t label_refs);


static void label_refs_visitor(void* parse, char* label, void* unused)
{
    prs_result_t* result = parse;
    long value;

    int r = hmap_get(result->labels, label, (void**)&value);
    if (r != MAP_OK)
    {
        report(P_ERROR, "<unknown>", 0, "referenced label [%s] is missing", label);
        result->consistent = -1;
    }
}


prs_result_t* parse_asm(char* filename, list_t* include_paths)
{
    char filepath[FILEPATH_BUF_SZ];
    if (find_file(filename, include_paths, filepath, FILEPATH_BUF_SZ) < 0)
    {
        report(P_ERROR, "<root>", 0, "file not found: %s", filename);
        return NULL;
    }

    prs_result_t* result = init_parse_result();

    hashset_t label_refs = hset_create();

    parse_asm_into(filename, include_paths, result, label_refs);

    hmap_iterate(label_refs, result, label_refs_visitor);

    hset_destroy(label_refs);
    return result;
}


static void free_label(void* unused, char* key, void* value)
{
    free(key);
}

void destroy_parse_result(prs_result_t* parse_result)
{
    for (uint32_t i = 0; i < parse_result->n_instns; i++)
    {
        destroy_instn(parse_result->instns[i]);
    }
    free(parse_result->instns);

    hmap_iterate(parse_result->labels, NULL, free_label);
    hmap_destroy(parse_result->labels);

    free(parse_result);
}


static void report(int kind, const char* source, uint32_t line, char* message, ...)
{
    va_list ap;

    fprintf(stderr, "%s [%s@%u]: ", ReportKinds[kind], source, line);

    va_start(ap, message);
    vfprintf(stderr, message, ap);
    va_end(ap);

    fprintf(stderr, "\n");
}


static prs_result_t* init_parse_result()
{
    prs_result_t* res = malloc(sizeof(prs_result_t));
    res->n_instns = 0;
    res->instns = NULL;
    res->labels = hmap_create();
    res->consistent = 0;
    return res;
}


static void destroy_instn(prs_instn_t* instn)
{
    for (int i = 0; i < instn->instn->arg_count; i++)
    {
        if (instn->args[i].type == T_ARG_REF_LBL)
        {
            free(instn->args[i].data.label);
        }
    }
    free(instn->args);
    free(instn);
}


static int find_file(const char* filename, list_t* include_paths,
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


static void parse_asm_into(const char* filename, list_t* include_paths,
    prs_result_t* result, hashset_t label_refs)
{
    FILE* fp = fopen(filename, "r");

    char* line = NULL;
    size_t sz = 0;
    uint32_t lineno = 0;
    while (getline(&line, &sz, fp) > 0)
    {
        lineno++;
        parse_line(filename, lineno, line, include_paths, result, label_refs);
        free(line);
        line = NULL;
        sz = 0;
    }

    free(line);
    fclose(fp);
}


static void parse_line(const char* filename, uint32_t lineno, char* line,
    list_t* include_paths, prs_result_t* result, hashset_t label_refs)
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

    int parse_instn = 1, tkn = 0;
    for (int mark_token = 0; *p; p++)
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
        add_instn(filename, lineno, (const char**)tokens, tkn + 1,
            result, label_refs);
    }
}


static void add_label(const char* label_token, prs_result_t* result)
{
    hmap_put(result->labels, strdup(label_token), (void*)(long)result->n_instns);
}


static void add_instn(const char* filename, uint32_t lineno, const char** tokens,
    int n_tokens, prs_result_t* result, hashset_t label_refs)
{
    prs_instn_t* instn = NULL;
    parse_instn(filename, lineno, tokens, n_tokens, &instn, label_refs);
    if (instn)
    {
        result->n_instns++;
        result->instns = realloc(result->instns, result->n_instns * sizeof(prs_instn_t*));

        result->instns[result->n_instns - 1] = instn;
    } else {
        result->consistent = -1;
    }
}


static void parse_instn(const char* filename, uint32_t lineno,
	const char** tokens, int n_tokens, prs_instn_t** result, hashset_t label_refs)
{
	instn_def_t* fmt = bsearch(&tokens[0], InstnDefs, nInstnDefs,
		sizeof(instn_def_t), compare_instn_def);
	if (!fmt)
	{
        report(P_ERROR, filename, lineno, "unknown instn: [%s]", tokens[0]);
		return;
	}

	// find the first instn which matches the token:
	while (fmt > InstnDefs && strcmp(tokens[0], (fmt - 1)->name) == 0)
	{
		fmt--;
	}

	// search for the matching instn:
	for (; strcmp(tokens[0], fmt->name) == 0; fmt++)
	{
		if (fmt->arg_count == n_tokens - 1)
		{
            prs_instn_t* instn = malloc(sizeof(prs_instn_t));
            instn->instn = fmt;
            instn->args = malloc(fmt->arg_count * sizeof(prs_arg_t));
            instn->mem_offset = (uint32_t)-1;

			for (int i = 1; i < n_tokens; i++)
			{
				if (parse_arg(filename, lineno, tokens[i], i - 1, &instn->args[i - 1], label_refs) < 0)
				{
                    destroy_instn(instn);
					return; // don't continue past first error in args
                }

                if (i == 1 && instn->args[0].type == T_ARG_IMM && (fmt->flags & F_ALLOW_IMM_1st_Arg) == 0)
                {
                    report(P_ERROR, filename, lineno, "instn [%s] disallows 1st arg to be imm", tokens[0]);
                    destroy_instn(instn);
                    return; // don't continue past first error in args
                }
            }

            *result = instn;
			return;
		}
	}

    report(P_ERROR, filename, lineno, "incorrect argument count for: [%s], found: %d", tokens[0], n_tokens - 1);
}


#define IS_1_BYTES_LONG(x)	(((x) & (~((uint64_t)0xff))) == 0)
#define IS_2_BYTES_LONG(x)	(((x) & (~((uint64_t)0xffff))) == 0)
#define IS_4_BYTES_LONG(x)	(((x) & (~((uint64_t)0xffffffff))) == 0)


static int parse_arg(const char* filename, uint32_t lineno,
	const char* token, int arg_index, prs_arg_t* result, hashset_t label_refs)
{
	char const* p = token;
	char* endp = NULL;
	int negative = 0, reference = 0;
	int base = 0;

	if (*p == '-')
	{
		negative = 1;
		p++;
	}

	if (*p == '@')
	{
		reference = 1;
		p++;
	}

	if (p[0] == '0' && p[1] == 'x') base = 16;

	uint64_t imm = strtoull(p, &endp, base);
	if ((imm == UINT64_MAX && errno == ERANGE) ||
		(imm > (INT64_MAX - 1) && negative))
	{
        report(P_ERROR, filename, lineno, "argument (%d) overflow: [%s]", arg_index, token);
		return -1;
	}
	if (!negative && imm > (INT64_MAX - 1))
	{
        report(P_WARN, filename, lineno, "argument (%d) overflows to negative value: [%s]", arg_index, token);
	}

	if (endp == p) // treat p as a label:
	{
		if (negative || reference)
		{
            report(P_ERROR, filename, lineno, "argument (%d) unexpected token after -/@", arg_index);
			return -1;
		}

        result->type = T_ARG_REF_LBL;
        result->data.label = strdup(p);
        hset_add(label_refs, result->data.label);
		return 0;
	}

	if (*endp != 0)
	{
        report(P_ERROR, filename, lineno, "argument (%d) bad token, expcted number", arg_index);
		return -1;
	}

	if (negative) imm = (uint64_t)(-(int64_t)imm);
	if (reference)
	{
        result->type = T_ARG_REF_NUM;
        result->data.value = imm;
		return 0;
	}

	int n_bytes = 8;
	if (!negative)
	{
		if (IS_1_BYTES_LONG(imm)) n_bytes = 1;
		else if (IS_2_BYTES_LONG(imm)) n_bytes = 2;
		else if (IS_4_BYTES_LONG(imm)) n_bytes = 4;
	}

    result->type = T_ARG_IMM;
    result->n_bytes = n_bytes;
    result->data.value = imm;
    return 0;
}
