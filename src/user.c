//
// Created by Rousan Ali on 24-10-2016.
//


#include "include/user.h"
#include "include/utils.h"

void bv_free_user_obj(bv_user* user_ptr) {
    bv_free((void*)user_ptr->username);
    bv_free((void*)user_ptr->enc_key);
    bv_free((void*)user_ptr->hints);
    bv_free((void*)user_ptr->password);
    bv_free((void*)user_ptr->sec_ques);
    bv_free((void*)user_ptr->sec_ques_ans);
}

void bv_free_byte_data_obj(bv_data* byte_data_ptr) {
    bv_free((void*)byte_data_ptr->key);
    bv_free((void*)byte_data_ptr->value);
}


void bv_free_byte_obj(bv_byte* byte_ptr) {
    size_t i = 0;

    for(i = 0; i<byte_ptr->data_count; i++) {
        bv_free((void*)byte_ptr->data[i].key);
        bv_free((void*)byte_ptr->data[i].value);
    }
    if(byte_ptr->data != null) {
        bv_free(byte_ptr->data);
    }
    bv_free((void*)byte_ptr->name);
    bv_free((void*)byte_ptr->last_modifying_text);
}



















