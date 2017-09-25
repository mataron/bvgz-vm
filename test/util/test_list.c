#include "../tests.h"
#include "util/list.h"


int main(int argc, char** argv) {
    list_t* a = mk_list_node(1);
    list_t* b = mk_list_node(2);
    list_t* c = mk_list_node(3);

    assert(list_size(a) == 1);
    assert(list_size(b) == 1);

    list_t* nd = list_prepend(a, b);

    assert(nd == b);

    assert(list_size(a) == 1);
    assert(list_size(b) == 2);

    nd = list_append(a, c);

    assert(nd == a);

    assert(list_size(b) == 3);
    assert(list_size(a) == 2);
    assert(list_size(c) == 1);

    assert(list_head(a) == list_head(b));
    assert(list_head(c) == list_head(b));
    assert(b == list_head(b));

    assert(list_tail(a) == list_tail(b));
    assert(list_tail(c) == list_tail(b));
    assert(c == list_tail(b));

    list_destroy_node(a);

    assert(list_size(b) == 2);
    assert(list_size(c) == 1);

    list_destroy_node(c);

    assert(list_size(b) == 1);

    list_destroy_node(b);
    return 0;
}
