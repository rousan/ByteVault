//
// Created by Rousan Ali on 06-10-2016.
//

#ifndef BYTEVAULT_VIEWER_H
#define BYTEVAULT_VIEWER_H

#include "common.h"
#include "user.h"
#include "linked_list.h"

#define BV_PIPE_CHAR 179
#define BV_HIFEN_CHAR 196
#define BV_PERPND_T_CHAR 195
#define BV_L_CHAR 192
#define BV_SPACE_CHAR 32
#define BV_BYTE_KEY_COLOR 0x6
#define BV_BYTE_VALUE_COLOR 0x2
#define BV_BYTE_NORMAL_COLOR 0x7
#define BV_BYTE_NAME_COLOR 0x7
#define BV_ERROR_COLOR 0x4
#define BV_LOG_COLOR 0x5
#define BV_WARNING_COLOR 0xC
#define BV_NORMAL_COLOR 0x7
#define BV_INPUT_STRING_KEY_COLOR 0x6
#define BV_INPUT_STRING_VALUE_COLOR 0x2

#define BV_SYM_COUNT 4


void bv_print_bytes(const char* title, bv_node* head_node, const char* flag);
void bv_print_user_info(const char* title, bv_user* user, size_t byte_count, size_t bytes_data_count);
void bv_apply_input_string_key_color();
void bv_apply_input_string_value_color();
void bv_apply_normal_color();
void bv_console_text_color(int color_code);
void bv_print_char(int chr, int count);
void bv_apply_byte_key_color();
void bv_apply_byte_value_color();
void bv_apply_byte_name_color();
void bv_format_date(time_t time_in_secs, char* dest);

#endif //BYTEVAULT_VIEWER_H
















