#include "tests.h"
#include "instn.h"
#include "parser.h"


static void test_labels()
{
    prs_result_t* result = parse_asm(
        xstr(PROJECT_ROOT) "/test/asm/labels-only.s", NULL);
    assert(result->errors == 0);
    assert(result->warnings == 0);
    assert(result->n_instns == 0);
    assert(hmap_size(result->labels) == 3);

    label_t* value;
    int r = hmap_get(result->labels, "abc", (void**)&value);
    assert(r == MAP_OK);
    assert(value->offset == 0);
    assert(value->is_mem_ref == 0);
    r = hmap_get(result->labels, "a000", (void**)&value);
    assert(r == MAP_OK);
    assert(value->offset == 0);
    assert(value->is_mem_ref == 0);
    r = hmap_get(result->labels, "__zz", (void**)&value);
    assert(r == MAP_OK);
    assert(value->offset == 0);
    assert(value->is_mem_ref == 0);

    assert(result->consistent == 0);

    destroy_parse_result(result);
}


static void test_instns()
{
    prs_instn_t* instn;

    prs_result_t* result = parse_asm(
        xstr(PROJECT_ROOT) "/test/asm/instn-only.s", NULL);
    assert(result->errors == 3);
    assert(result->warnings == 0);
    assert(result->n_instns == 3);
    assert(hmap_size(result->labels) == 0);

    instn = result->instns[0];
    assert(instn->instn->arg_count == 2);
    assert(instn->args[0].type == T_ARG_REF_LBL);
    assert(strcmp(instn->args[0].data.label, "x") == 0);
    assert(instn->args[1].type == T_ARG_REF_LBL);
    assert(strcmp(instn->args[1].data.label, "y") == 0);

    instn = result->instns[1];
    assert(instn->instn->arg_count == 3);
    assert(instn->args[0].type == T_ARG_REF_LBL);
    assert(strcmp(instn->args[0].data.label, "z") == 0);
    assert(instn->args[1].type == T_ARG_IMM);
    assert(instn->args[1].data.value == 1);
    assert(instn->args[1].n_bytes == 1);
    assert(instn->args[2].type == T_ARG_IMM);
    assert(instn->args[2].data.value == -34);
    assert(instn->args[2].n_bytes == 8);

    instn = result->instns[2];
    assert(instn->instn->arg_count == 3);
    assert(instn->args[0].type == T_ARG_REF_NUM);
    assert(instn->args[0].data.value == 34);
    assert(instn->args[1].type == T_ARG_IMM);
    assert(instn->args[1].data.value == -0x9090909090);
    assert(instn->args[1].n_bytes == 8);
    assert(instn->args[2].type == T_ARG_IMM);
    assert(instn->args[2].data.value == 0x2345);
    assert(instn->args[2].n_bytes == 2);

    assert(result->consistent == -1);

    destroy_parse_result(result);
}


static void test_include()
{
    list_t* include_paths = list_make_node(
        xstr(PROJECT_ROOT) "/test/asm/foo");
    prs_result_t* result = parse_asm(
        xstr(PROJECT_ROOT) "/test/asm/include.s", include_paths);

    assert(result->errors == 0); // ensures inclusions are OK
    assert(result->warnings == 0);

    destroy_parse_result(result);
    list_destroy_node(include_paths);
}


static void test_instns_w_labels()
{
    prs_result_t* result = parse_asm(
        xstr(PROJECT_ROOT) "/test/asm/instn-w-labels.s", NULL);
    assert(result->errors == 0);
    assert(result->warnings == 0);
    assert(result->n_instns == 3);
    assert(hmap_size(result->labels) == 3);
    assert(result->consistent == 0);

    destroy_parse_result(result);
}


static void test_data_loads()
{
    prs_result_t* result = parse_asm(
        xstr(PROJECT_ROOT) "/test/asm/data.s", NULL);
    assert(result->errors == 0);
    assert(result->warnings == 1);
    assert(result->n_instns == 1);
    assert(hmap_size(result->labels) == 3);
    assert(result->consistent == 0);

    printf("memsz = %u\n", result->memsz);
    // true total is 226, but 12 bytes are removed due to specific size
    assert(result->memsz == 214);

    destroy_parse_result(result);
}


static void test_instns_lookup()
{
    instn_def_t* fmt;

    const char* str = "mod";
    fmt = bsearch(&str, InstnDefs, nInstnDefs,
        sizeof(instn_def_t), compare_instn_def);
    assert(fmt != NULL);

    str = "add";
    fmt = bsearch(&str, InstnDefs, nInstnDefs,
        sizeof(instn_def_t), compare_instn_def);
    assert(fmt != NULL);

    str = "yield";
    fmt = bsearch(&str, InstnDefs, nInstnDefs,
        sizeof(instn_def_t), compare_instn_def);
    assert(fmt != NULL);
}


int main(int argc, char** argv)
{
    setup_instn_defs();

    test_instns_lookup();
    test_labels();
    test_instns();
    test_instns_w_labels();
    test_include();
    test_data_loads();

    return 0;
}
