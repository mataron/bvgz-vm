#include <stdlib.h>

#include "list.h"


list_t* list_make_node(void* data)
{
    list_t* node = malloc(sizeof(list_t));
    node->prev = node->next = NULL;
    node->data = data;
    return node;
}


void list_destroy_node(list_t* node)
{
    list_unlink(node);
    free(node);
}


void list_destroy(list_t* head)
{
    list_t* node = head;
    list_t* next = head ? head->next : NULL;
    while (node)
    {
        free(node);
        node = next;
        next = node ? node->next : NULL;
    }
}


void list_unlink(list_t* node) {
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
}


list_t* list_append(list_t* list, list_t* node)
{
    if (list == NULL) return node;
    node->next = list->next;
    if (list->next) list->next->prev = node;
    list->next = node;
    node->prev = list;
    return list;
}


list_t* list_prepend(list_t* list, list_t* node)
{
    if (list == NULL) return node;
    node->prev = list->prev;
    if (list->prev) list->prev->next = node;
    list->prev = node;
    node->next = list;
    return node;
}


uint32_t list_size(list_t* head)
{
    uint32_t size = 0;
    for (list_t* n = head; n; n = n->next)
    {
        size++;
    }
    return size;
}


list_t* list_head(list_t* node)
{
    while (node->prev) node = node->prev;
    return node;
}


list_t* list_tail(list_t* node)
{
    while (node->next) node = node->next;
    return node;
}
