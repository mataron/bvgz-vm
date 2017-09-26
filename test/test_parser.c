#include "tests.h"
#include "instn.h"
#include "parser.h"


static void test_labels()
{
    prs_result_t* result = parse_asm(xstr(PROJECT_ROOT) "/test/asm/labels-only.s", NULL);
    destroy_parse_result(result);
}


int main(int argc, char** argv)
{
    setup_instn_defs();

    test_labels();

    return 0;
}
