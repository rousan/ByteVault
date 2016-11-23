//
// Created by Ariyan Khan on 09-10-2016.
//

#ifndef BYTEVAULT_LINKED_LIST_H
#define BYTEVAULT_LINKED_LIST_H

#include "common.h"

typedef struct bv_node{
    void* value_ptr;
    struct bv_node* next;
} bv_node;


void bv_ll_add_node(bv_node *head, bv_node *new_node);
size_t bv_ll_count(bv_node* head);
bv_node* bv_ll_get(bv_node* head, size_t pos);
bv_node* bv_ll_search_by_value(bv_node* head, void* value_ptr);

#endif //BYTEVAULT_LINKED_LIST_H
