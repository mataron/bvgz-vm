#ifndef _BVGZ_LIST_H
#define _BVGZ_LIST_H

#include <stdint.h>

typedef struct _list_t
{
    struct _list_t* prev;
    struct _list_t* next;

    void* data;
}
list_t;


#define mk_list_node(x)  list_make_node((void*)(x))

list_t* list_make_node(void* data);
void list_destroy_node(list_t* node);
void list_unlink(list_t* node);

list_t* list_append(list_t* list, list_t* node);
list_t* list_prepend(list_t* list, list_t* node);

uint32_t list_size(list_t* head);

list_t* list_head(list_t* node);
list_t* list_tail(list_t* node);

#endif
