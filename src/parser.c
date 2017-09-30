#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

#include "util/hashset.h"
#include "parser.h"
#include "instn.h"

#define FILEPATH_BUF_SZ     256

#ifndef NDEBUG
#define MEM_ALLOC_BLOCK_SZ  10
#else
#define MEM_ALLOC_BLOCK_SZ  512
#endif

// maximum number of tokens per instn line
#define MAX_TOKENS          4

#define P_ERROR 0
#define P_WARN  1

#define PREPROC_DATA    "%data"
#define PREPROC_INCL    "%include"

static char* ReportKinds[] = {
    "Error",
    "Warning"
};

static void report(prs_result_t* r, int kind, const char* source,
    uint32_t line, char* message, ...);

static prs_result_t* init_parse_result();
static void destroy_instn(prs_instn_t*);

static int find_file(const char* filename, list_t* include_paths,
    char* filepath, int filepath_sz);
static void parse_asm_into(const char* filename, list_t* include_paths,
    prs_result_t* result, hashset_t label_refs);

static void parse_line(const char* filename, uint32_t lineno, char* line,
    list_t* include_paths, prs_result_t* result, hashset_t label_refs);

static void parse_preproc_line(const char* filename, uint32_t lineno,
    char* line, prs_result_t* result, list_t* include_paths,
    hashset_t label_refs);
static void parse_preproc_data(const char* filename, uint32_t lineno,
    char* line, list_t* include_paths, prs_result_t* result);
static void parse_preproc_include(const char* filename, uint32_t lineno,
    char* line, prs_result_t* result, list_t* include_paths,
    hashset_t label_refs);

static void add_label(const char* label_token,
    prs_result_t* result, uint32_t offset, uint8_t is_mem_ref);
static void add_instn(const char* filename, uint32_t lineno,
    const char** tokens, int n_tokens, prs_result_t* result,
    hashset_t label_refs);

static void parse_instn(const char* filename, uint32_t lineno,
    const char** tokens, int n_tokens, prs_result_t* result,
    prs_instn_t** instn, hashset_t label_refs);
static int parse_arg(const char* filename, uint32_t lineno,
    const char* token, int arg_index, prs_result_t* result,
    prs_arg_t* arg, hashset_t label_refs);


static void label_refs_visitor(void* parse, char* label, void* unused)
{
    prs_result_t* result = parse;
    label_t* label_val;

    int r = hmap_get(result->labels, label, (void**)&label_val);
    if (r != MAP_OK)
    {
        report(result, P_ERROR, "<top-level>", 0,
            "referenced label [%s] is missing", label);
        result->consistent = -1;
    }
}


prs_result_t* parse_asm(char* filename, list_t* include_paths)
{
    char filepath[FILEPATH_BUF_SZ];
    struct stat st;
    int free_first_path = 0;
    if (stat(filename, &st) == 0)
    {
        strncpy(filepath, filename, FILEPATH_BUF_SZ);
        char* slash = strrchr(filename, '/');
        if (slash)
        { // copy the directory of the root file.
            list_t* head = list_make_node(
                strndup(filename, slash - filename));
            include_paths = list_prepend(include_paths, head);
            free_first_path = 1;
        }
    }
    else if (find_file(filename, include_paths, filepath,
                FILEPATH_BUF_SZ) < 0)
    {
        report(NULL, P_ERROR, "<root>", 0,
            "file not found: %s", filename);
        return NULL;
    }

    prs_result_t* result = init_parse_result();

    hashset_t label_refs = hset_create();

    parse_asm_into(filename, include_paths, result, label_refs);

    hmap_iterate(label_refs, result, label_refs_visitor);

    hset_destroy(label_refs);

    if (free_first_path)
    {
        free(include_paths->data);
        list_destroy_node(include_paths);
    }
    return result;
}


uint32_t resolve_entry_point(char* entry_label, prs_result_t* result)
{
    label_t* elbl = NULL;

    if (hmap_get(result->labels, entry_label, (void**)&elbl) != MAP_OK)
    {
        fprintf(stderr, "entry label not found: %s\n",
        entry_label);
        return (uint32_t)-1;
    }

    if (elbl->is_mem_ref)
    {
        fprintf(stderr, "entry label refers to memory segment: %s\n",
            entry_label);
        return (uint32_t)-1;
    }

    return elbl->offset;
}


static void free_label(void* unused, char* key, void* value)
{
    free(key);
    free(value);
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

    free(parse_result->memory);

    free(parse_result);
}


static void report(prs_result_t* r, int kind, const char* source,
    uint32_t line, char* message, ...)
{
    if (r)
    {
        if (kind == P_ERROR) r->errors++;
        else if (kind == P_WARN) r->warnings++;
    }

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
    res->errors = res->warnings = 0;
    res->n_instns = 0;
    res->instns = NULL;
    res->memory = NULL;
    res->memsz = 0;
    res->mem_alloc = 0;
    res->labels = hmap_create();
    res->consistent = 0;
    return res;
}


static void destroy_instn(prs_instn_t* instn)
{
    for (int i = 0; i < instn->instn->arg_count; i++)
    {
        uint8_t type = instn->args[i].type;
        if (type == T_ARG_REF_LBL || type == T_ARG_IMM_LBL)
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
    for (list_t* p = include_paths; p; p = p->next)
    {
        snprintf(filepath, filepath_sz, "%s/%s",
            (char*)p->data, filename);
        if (stat(filepath, &st) == 0) return 0;
    }

    return -1;
}


static void parse_asm_into(const char* filename, list_t* include_paths,
    prs_result_t* result, hashset_t label_refs)
{
    FILE* fp = fopen(filename, "rt");

    char* line = NULL;
    size_t sz = 0;
    uint32_t lineno = 0;
    while (getline(&line, &sz, fp) > 0)
    {
        lineno++;
        parse_line(filename, lineno, line, include_paths, result,
            label_refs);
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

    if (*p == '%')
    {
        parse_preproc_line(filename, lineno, line, result, include_paths,
            label_refs);
        return;
    }

    tokens[0] = p;

    int parse_instn = 1, tkn = 0;
    for (int mark_token = 0; *p; p++)
    {
        if (*p == ';')
        {
            *p = 0;
            break;
        }
        else if (isspace(*p) || *p == ',')
        {
            *p = 0;
            mark_token = 1;
        }
        else if (tkn == 0 && *p == ':')
        {
            *p = 0;
            add_label(tokens[0], result, result->n_instns, 0);
            parse_instn = 0;
        }
        else if (!parse_instn)
        {
            report(result, P_ERROR, filename, lineno,
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


static void parse_preproc_line(const char* filename, uint32_t lineno,
    char* line, prs_result_t* result, list_t* include_paths,
    hashset_t label_refs)
{
    if (strncmp(line, PREPROC_DATA, sizeof(PREPROC_DATA) - 1) == 0)
    {
        parse_preproc_data(filename, lineno, line, include_paths,
            result);
    }
    else if (strncmp(line, PREPROC_INCL, sizeof(PREPROC_INCL) - 1) == 0)
    {
        parse_preproc_include(filename, lineno, line, result,
            include_paths, label_refs);
    }
}


static uint32_t parse_preproc_data_no(const char* filename,
    uint32_t lineno, char* numeric_str, const char* data_name,
    prs_result_t* result)
{
    char* end = NULL;
    uint32_t n = strtoul(numeric_str, &end, 10);
    if (*end && *end != ';')
    {
        report(result, P_ERROR, filename, lineno,
            "bad data specification: [%s]: base 10 integer expected",
            data_name);
        return 0;
    }
    return n;
}


static void ensure_memory_alloc(uint32_t alloc, prs_result_t* result)
{
    if (alloc > result->memsz)
    {
        uint32_t new_size = result->mem_alloc;
        while (alloc > new_size) new_size += MEM_ALLOC_BLOCK_SZ;
        if (new_size > result->mem_alloc)
        {
            result->memory = realloc(result->memory, new_size);
            memset(result->memory + result->mem_alloc, 0,
                new_size - result->mem_alloc);
            result->mem_alloc = new_size;
        }
    }
}


static void ensure_memory_at_offset(uint32_t size, uint32_t offset,
    prs_result_t* result)
{
    if (offset == (uint32_t)-1) return;

    uint32_t min_alloc = offset + (size ? size : 1);
    ensure_memory_alloc(min_alloc, result);
}


static void write_to_memory(uint8_t* src, uint32_t size,
    uint32_t mem_offset, prs_result_t* result)
{
    ensure_memory_alloc(mem_offset + size, result);
    memcpy(result->memory + mem_offset, src, size);
    result->memsz = mem_offset + size;
}


static void set_memory_size(const char* filename, uint32_t lineno,
    uint32_t original_offset, uint32_t size, prs_result_t* result)
{
    if (size != (uint32_t)-1)
    {
        uint32_t total = size + original_offset;
        if (total < result->memsz)
        {
            report(result, P_WARN, filename, lineno,
                "trimming memory size %u -> %u bytes",
                result->memsz, total);
        }
        else if (total > result->memsz)
        {
            ensure_memory_alloc(total, result);
        }
        result->memsz = total;
    }
}


static void add_file_to_memory(const char* filename, uint32_t lineno,
    char* label, char* incl_file, uint32_t size, uint32_t offset,
    list_t* include_paths, prs_result_t* result)
{
    char filepath[FILEPATH_BUF_SZ];
    if (find_file(incl_file, include_paths, filepath,
            FILEPATH_BUF_SZ) < 0)
    {
        report(result, P_ERROR, filename, lineno,
            "file not found: %s", incl_file);
        return;
    }

    ensure_memory_at_offset(size, offset, result);

    uint8_t buffer[MEM_ALLOC_BLOCK_SZ];
    FILE* fp = fopen(filepath, "rt");

    size_t sz;
    uint32_t w_offset = offset != (uint32_t)-1 ? offset : result->memsz;
    uint32_t original_offset = w_offset;
    while((sz = fread(buffer, 1, MEM_ALLOC_BLOCK_SZ, fp)) > 0)
    {
        write_to_memory(buffer, sz, w_offset, result);
        w_offset += sz;
    }

    fclose(fp);

    set_memory_size(filename, lineno, original_offset, size, result);
    if (label)
    {
        add_label(label + 1 /* skip '@' chr */, result, original_offset, 1);
    }
}


static void add_str_to_memory(const char* filename, uint32_t lineno,
    char* label, char* str, uint32_t size, uint32_t offset,
    prs_result_t* result)
{
    ensure_memory_at_offset(size, offset, result);

    uint32_t w_offset = offset != (uint32_t)-1 ? offset : result->memsz;
    uint32_t original_offset = w_offset;
    int escape_chr = 0, stop = 0;
    char* p;
    uint8_t tmp;
    for (p = str + 1; *p && !stop; p++)
    {
        switch (*p)
        {
        case '\\':
            if (escape_chr)
            {
                escape_chr = 0;
                write_to_memory((uint8_t*)p, 1, w_offset, result);
                w_offset++;
                break;
            }
            escape_chr = 1;
            break;

        case '"':
            if (escape_chr)
            {
                escape_chr = 0;
                write_to_memory((uint8_t*)p, 1, w_offset, result);
                w_offset++;
                break;
            }
            stop = 1;
            break;

#define WRITE_CHR_OR_ESCAPED(escaped)   \
            if (escape_chr)\
            {\
                escape_chr = 0;\
                tmp = escaped;\
                write_to_memory(&tmp, 1, w_offset, result);\
                w_offset++;\
                break;\
            }\
            write_to_memory((uint8_t*)p, 1, w_offset, result);\
            w_offset++;\
            break;

        case 'a': WRITE_CHR_OR_ESCAPED(0x07);
        case 'b': WRITE_CHR_OR_ESCAPED(0x08);
        case 'f': WRITE_CHR_OR_ESCAPED(0x0C);
        case 'n': WRITE_CHR_OR_ESCAPED(0x0A);
        case 'r': WRITE_CHR_OR_ESCAPED(0x0D);
        case 't': WRITE_CHR_OR_ESCAPED(0x09);
        case 'v': WRITE_CHR_OR_ESCAPED(0x0B);

#undef WRITE_CHR_OR_ESCAPED
        default:
            write_to_memory((uint8_t*)p, 1, w_offset, result);
            w_offset++;
        }
    }

    // NULL terminate the string
    tmp = 0;
    write_to_memory(&tmp, 1, w_offset, result);

    if (!stop)
    {
        report(result, P_ERROR, filename, lineno, "bad string format");
    }

    set_memory_size(filename, lineno, original_offset, size, result);
    if (label)
    {
        add_label(label + 1 /* skip '@' chr */, result, original_offset, 1);
    }
}


static void add_hex_to_memory(const char* filename, uint32_t lineno,
    char* label, char* hex, uint32_t size, uint32_t offset,
    prs_result_t* result)
{
    ensure_memory_at_offset(size, offset, result);

    char* end = NULL;
    uint64_t n = strtoull(hex + 2, &end, 16);

    if (*end)
    {
        report(result, P_WARN, filename, lineno, "bad hex value");
    }

    uint32_t w_offset = offset != (uint32_t)-1 ? offset : result->memsz;

    size_t len = strlen(hex + 2); // skip '0x'
#define WRITE_BITS(bits)\
        {\
            uint ## bits ## _t tmp = n;\
            write_to_memory((uint8_t*)&tmp, bits / 8, w_offset,\
                result);\
            break;\
        }

    switch (len / 2 + (len & 1))
    {
        case 1: WRITE_BITS(8);
        case 2: WRITE_BITS(16);
        case 4: WRITE_BITS(32);
        case 8: WRITE_BITS(64);
        default:
            report(result, P_WARN, filename, lineno,
                "cannot determine number length: default to 64 bits");
            WRITE_BITS(64);
    }

#undef WRITE_BITS

    set_memory_size(filename, lineno, w_offset, size, result);
    if (label)
    {
        add_label(label + 1 /* skip '@' chr */, result, w_offset, 1);
    }
}


static void parse_string_literal(char* token, char** p)
{
    char* c = token + 1;
    char prev = 0;
    while (*c)
    {
        if (*c == '"' && prev != '\\')
        {
            c++;
            break;
        }

        prev = *c;
        c++;
    }
    *c = 0;
    *p = c;
}


static void parse_preproc_data(const char* filename, uint32_t lineno,
    char* line, list_t* include_paths, prs_result_t* result)
{
    uint32_t size = (uint32_t)-1;
    uint32_t offset = (uint32_t)-1;
    char* label = NULL;
    char* data = NULL;
    char* p = NULL;
    char p_value = 0;
    char* token = line + sizeof(PREPROC_DATA);

#define NEXT_TOKEN(required) \
    while (*token && isspace(*token)) token++;\
    p = token;\
    while (*p && !isspace(*p) && *p != ';') p++;\
    if (required && (token == p || !*p))\
    {\
        report(result, P_ERROR, filename, lineno,\
            "bad data directive: incorrect specification");\
        return;\
    }\
    if (*token == ';') *token = 0;\
    p_value = *p;\
    *p = 0;

    NEXT_TOKEN(1);

    if (*token == '@')
    {
        label = token;
        token = p + 1;
        NEXT_TOKEN(1);
    }

    data = token;
    if (*data == '"')
    {
        *p = p_value;
        parse_string_literal(token, &p);
    }
    token = p + 1;

    while (1)
    {
        NEXT_TOKEN(0);
        if (!*token) break;

        if (*token == ':')
        {
            token++;

            if (offset != (uint32_t)-1)
            {
                report(result, P_ERROR, filename, lineno,
                    "multiple offset definitions");
            }

            offset = parse_preproc_data_no(filename, lineno, token,
                "offset", result);

            if (offset && offset < result->memsz)
            {
                report(result, P_WARN, filename, lineno,
                    "data collide with previous allocations: %u bytes",
                    result->memsz - offset);
            }

            token = p + 1;
        }
        else if (*token == '/')
        {
            token++;

            if (offset != (uint32_t)-1)
            {
                report(result, P_ERROR, filename, lineno,
                    "multiple size definitions");
            }

            size = parse_preproc_data_no(filename, lineno, token,
                "size", result);

            token = p + 1;
        }
        else
        {
            report(result, P_ERROR, filename, lineno,
                "unexpected token: %s", token);
            break;
        }
    }

#undef NEXT_TOKEN

    if (*data == '=')
    {
        add_file_to_memory(filename, lineno, label, data + 1, size,
            offset, include_paths, result);
    }
    else if (*data == '"')
    {
        add_str_to_memory(filename, lineno, label, data, size,
            offset, result);
    }
    else if (*data == '0' && *(data + 1) == 'x' && *(data + 2))
    {
        add_hex_to_memory(filename, lineno, label, data, size,
            offset, result);
    }
    else
    {
        report(result, P_ERROR, filename, lineno,
            "bad data directive: incorrect data specification");
        return;
    }

    assert(result->memsz <= result->mem_alloc);
}


static void parse_preproc_include(const char* filename, uint32_t lineno,
    char* line, prs_result_t* result, list_t* include_paths,
    hashset_t label_refs)
{
    char* file = line + sizeof(PREPROC_INCL);
    while (*file && isspace(*file)) file++;

    char* p = file;
    while (*p && !isspace(*p) && *p != ';') p++;
    *p = 0;

    if (file == p)
    {
        report(result, P_ERROR, filename, lineno,
            "expected filename after %include directive");
        return;
    }

    char filepath[FILEPATH_BUF_SZ];
    if (find_file(file, include_paths, filepath, FILEPATH_BUF_SZ) < 0)
    {
        report(result, P_ERROR, filename, lineno,
            "file not found: %s", file);
        return;
    }

    parse_asm_into(filepath, include_paths, result, label_refs);
}


static void add_label(const char* label_token,
    prs_result_t* result, uint32_t offset, uint8_t is_mem_ref)
{
    label_t* label = malloc(sizeof(label_t));
    label->offset = offset;
    label->is_mem_ref = is_mem_ref;
    hmap_put(result->labels, strdup(label_token), label);
}


static void add_instn(const char* filename, uint32_t lineno,
    const char** tokens, int n_tokens, prs_result_t* result,
    hashset_t label_refs)
{
    prs_instn_t* instn = NULL;
    parse_instn(filename, lineno, tokens, n_tokens, result, &instn,
        label_refs);
    if (instn)
    {
        result->n_instns++;
        result->instns = realloc(result->instns,
            result->n_instns * sizeof(prs_instn_t*));

        result->instns[result->n_instns - 1] = instn;
    }
    else
    {
        result->consistent = -1;
    }
}


static void parse_instn(const char* filename, uint32_t lineno,
    const char** tokens, int n_tokens, prs_result_t* result,
    prs_instn_t** _instn, hashset_t label_refs)
{
	instn_def_t* fmt = bsearch(&tokens[0], InstnDefs, nInstnDefs,
		sizeof(instn_def_t), compare_instn_def);
	if (!fmt)
	{
        report(result, P_ERROR, filename, lineno,
            "unknown instn: [%s]", tokens[0]);
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
                if (parse_arg(filename, lineno, tokens[i], i - 1,
                        result, &instn->args[i - 1], label_refs) < 0)
				{
                    destroy_instn(instn);
					return; // don't continue past first error in args
                }

                if (i == 1 && instn->args[0].type == T_ARG_IMM &&
                    (fmt->flags & F_ALLOW_IMM_1st_Arg) == 0)
                {
                    report(result, P_ERROR, filename, lineno,
                        "instn [%s] disallows 1st arg to be imm",
                        tokens[0]);
                    destroy_instn(instn);
                    return; // don't continue past first error in args
                }
            }

            *_instn = instn;
			return;
		}
	}

    report(result, P_ERROR, filename, lineno,
        "incorrect argument count for: [%s], found: %d",
        tokens[0], n_tokens - 1);
}


#define IS_1_BYTES_LONG(x)	(((x) & (~((uint64_t)0xff))) == 0)
#define IS_2_BYTES_LONG(x)	(((x) & (~((uint64_t)0xffff))) == 0)
#define IS_4_BYTES_LONG(x)	(((x) & (~((uint64_t)0xffffffff))) == 0)


static int parse_arg(const char* filename, uint32_t lineno,
    const char* token, int arg_index, prs_result_t* result,
    prs_arg_t* arg, hashset_t label_refs)
{
	char const* p = token;
	char* endp = NULL;
	int negative = 0, reference = 0, imm_ref = 0;
	int base = 0;

	if (*p == '-')
	{
		negative = 1;
		p++;
	}

	if (*p == '*')
	{
		reference = 1;
		p++;
    }

    if (*p == '&')
    {
        imm_ref = 1;
        p++;
    }

	if (p[0] == '0' && p[1] == 'x') base = 16;

	uint64_t imm = strtoull(p, &endp, base);
	if ((imm == UINT64_MAX && errno == ERANGE) ||
		(imm > (INT64_MAX - 1) && negative))
	{
        report(result, P_ERROR, filename, lineno,
            "argument (%d) overflow: [%s]", arg_index, token);
		return -1;
	}
	if (!negative && imm > (INT64_MAX - 1))
	{
        report(result, P_WARN, filename, lineno,
            "argument (%d) overflows to negative value: [%s]",
            arg_index, token);
	}

	if (endp == p) // treat p as a label:
	{
		if (negative || reference)
		{
            report(result, P_ERROR, filename, lineno,
                "argument (%d) unexpected token after -/@", arg_index);
			return -1;
		}

        arg->type = imm_ref ? T_ARG_IMM_LBL : T_ARG_REF_LBL;
        arg->n_bytes = 4;
        arg->data.label = strdup(p);
        hset_add(label_refs, arg->data.label);
		return 0;
	}

	if (*endp != 0)
	{
        report(result, P_ERROR, filename, lineno,
            "argument (%d) bad token, expcted number", arg_index);
		return -1;
	}

	if (negative) imm = (uint64_t)(-(int64_t)imm);
	if (reference)
	{
        arg->type = T_ARG_REF_NUM;
        arg->n_bytes = 4;
        arg->data.value = imm;
		return 0;
	}

	int n_bytes = 8;
	if (!negative)
	{
		if (IS_1_BYTES_LONG(imm)) n_bytes = 1;
		else if (IS_2_BYTES_LONG(imm)) n_bytes = 2;
		else if (IS_4_BYTES_LONG(imm)) n_bytes = 4;
	}

    arg->type = T_ARG_IMM;
    arg->n_bytes = n_bytes;
    arg->data.value = imm;
    return 0;
}
