//
// Created by Ariyan Khan on 09-10-2016.
//

#include "include/linked_list.h"

void bv_ll_add_node(bv_node *head, bv_node *new_node) {
    bv_node* next_node = NULL;
    next_node = head;
    while (1) {
        if(next_node->next == NULL) {
            next_node->next = new_node;
            new_node->next = NULL;//It is very very important to remove circle reference adding(same node multiple adding)
            break;
        } else {
            next_node = next_node->next;
        }
    }
}

size_t bv_ll_count(bv_node* head) {
    if(head == NULL) {
        return 0;
    }
    size_t i = 1;
    while ((head = head->next) != NULL) {
        i++;
    }
    return i;
}

bv_node* bv_ll_get(bv_node* head, size_t pos) {
    size_t i = 0;
    bv_node* node_temp;

    node_temp = head;
    while ( node_temp != NULL ) {
        if(i == pos) {
            return node_temp;
        }
        i++;
        node_temp = node_temp->next;
    }
    return NULL;
}


bv_node* bv_ll_search_by_value(bv_node* head, void* value_ptr) {
    bv_node* node_temp;

    node_temp = head;
    while ( node_temp != NULL ) {
        if(((unsigned char*)node_temp->value_ptr) == ((unsigned char*)value_ptr) ) {
            return node_temp;
        }
        node_temp = node_temp->next;
    }
    return NULL;
}












