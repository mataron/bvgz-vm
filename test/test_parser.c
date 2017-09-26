#include "tests.h"
#include "instn.h"
#include "parser.h"


static void test_labels()
{
    prs_result_t* result = parse_asm(xstr(PROJECT_ROOT) "/test/asm/labels-only.s", NULL);
    assert(result->n_nodes == 3);
    assert(result->nodes[0]->type == PT_LABEL);
    assert(result->nodes[1]->type == PT_LABEL);
    assert(result->nodes[2]->type == PT_LABEL);
    destroy_parse_result(result);
}


static void test_instns()
{
    prs_instn_t* instn;

    prs_result_t* result = parse_asm(xstr(PROJECT_ROOT) "/test/asm/instn-only.s", NULL);
    assert(result->n_nodes == 3);

    assert(result->nodes[0]->type == PT_INSTN);
    instn = (prs_instn_t*)result->nodes[0];
    assert(instn->instn->arg_count == 2);
    assert(instn->args[0].type == T_ARG_REF_LBL);
    assert(strcmp(instn->args[0].data.label, "x") == 0);
    assert(instn->args[1].type == T_ARG_REF_LBL);
    assert(strcmp(instn->args[1].data.label, "y") == 0);

    assert(result->nodes[1]->type == PT_INSTN);
    instn = (prs_instn_t*)result->nodes[1];
    assert(instn->instn->arg_count == 3);
    assert(instn->args[0].type == T_ARG_REF_LBL);
    assert(strcmp(instn->args[0].data.label, "z") == 0);
    assert(instn->args[1].type == T_ARG_IMM);
    assert(instn->args[1].data.value == 1);
    assert(instn->args[1].n_bytes == 1);
    assert(instn->args[2].type == T_ARG_IMM);
    assert(instn->args[2].data.value == -34);
    assert(instn->args[2].n_bytes == 8);

    assert(result->nodes[2]->type == PT_INSTN);
    instn = (prs_instn_t*)result->nodes[2];
    assert(instn->instn->arg_count == 3);
    assert(instn->args[0].type == T_ARG_REF_NUM);
    assert(instn->args[0].data.value == 34);
    assert(instn->args[1].type == T_ARG_IMM);
    assert(instn->args[1].data.value == -0x9090909090);
    assert(instn->args[1].n_bytes == 8);
    assert(instn->args[2].type == T_ARG_IMM);
    assert(instn->args[2].data.value == 0x2345);
    assert(instn->args[2].n_bytes == 2);

    destroy_parse_result(result);
}


static void test_instns_lookup()
{
    instn_def_t* fmt;

    assert(nInstnDefs == 46);

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

    return 0;
}
