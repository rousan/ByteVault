//
// Created by Rousan Ali on 06-10-2016.
//

#ifndef BYTEVAULT_USER_H
#define BYTEVAULT_USER_H

#include "common.h"
#include <time.h>

typedef struct {
    const char* username;
    const char* password;
    const char* hints;
    const char* enc_key;
    const char* sec_ques;
    const char* sec_ques_ans;
} bv_user;

typedef struct {
    const char* key;
    const char* value;
} bv_data;

typedef struct {
    const char* name;
    const char* last_modifying_text;
    unsigned long long id;
    unsigned long long parent_byte_id; //its value will be 0 if there is no parent, and otherwise greater than 0.
    time_t date_last_modified;
    time_t date_created;
    bv_data* data;
    size_t data_count;
} bv_byte;

void bv_free_user_obj(bv_user* user_ptr);
void bv_free_byte_data_obj(bv_data* byte_data_ptr);
void bv_free_byte_obj(bv_byte* byte_ptr);


#endif //BYTEVAULT_USER_H

















