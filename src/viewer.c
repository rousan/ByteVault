//
// Created by Rousan Ali on 06-10-2016.
//

#include <windows.h>

#include "include/user.h"
#include "include/logger.h"
#include "include/viewer.h"
#include "include/utils.h"
#include "include/database.h"

static void bv_print_byte_deeps(size_t deep, bv_bool* deep_flags);
static void bv_print_single_byte(bv_bool end_byte, size_t deep, bv_bool* deep_flags, bv_byte* byte, int flags);

/*
 * It prints bytes with hierarchically all child bytes.
 * It does not check whose parent is null or not.
 * It just prints self byte and its childs, but not check its parent is null or not i.e.
 * Self is child of another or not.
 * Example:
 *  A is child of B and B is child of C and C is child of D.
 *  if in head_node parameter B is exist as node, then it will be printed and its child A, though its parent is C.
 * All IS HANDLED BY showbyte() function in command_actions.
 * To get all byte use "bv_get_direct_child_bytes" function
 */
void bv_print_bytes(const char* title, bv_node* head_node, const char* flag) {
    size_t i = 0;
    size_t node_count = 0;
    bv_bool end_of_byte = bv_false;
    int flags_int = 0;
    bv_byte* byte_temp;
    bv_node* node_temp = null;


    if(flag == null) {
        flags_int = 1;
    } else if(!strcmpi(flag, "-i")) {
        flags_int = 0;
    } else if(!strcmpi(flag, "-d")) {
        flags_int = 1;
    } else {
        flags_int = 1;
    }

    node_count = bv_ll_count(head_node);
    fprintf(stdout, "%s\n", title);
    for(i = 0; i<node_count; i++) {
        if(i == node_count - 1) {
            end_of_byte = bv_true;
        } else {
            end_of_byte = bv_false;
        }
        byte_temp = bv_ll_get(head_node, i)->value_ptr;
        bv_print_single_byte(end_of_byte, 0, null, byte_temp, flags_int);
    }

    while (head_node != null) {
        byte_temp = head_node->value_ptr;
        bv_free_byte_obj(byte_temp);
        free(byte_temp);
        node_temp = head_node;
        head_node = head_node->next;
        free(node_temp);
    }
}

static void bv_print_single_byte(bv_bool end_byte, size_t deep, bv_bool* deep_flags, bv_byte* byte, int flags) {
    size_t i = 0;
    bv_node* node_ptr_temp = null;
    int rc;
    char* errMsg = null;
    bv_byte* temp_byte_ptr = null;
    size_t child_bytes_count = 0;
    bv_bool current_end_of_byte = bv_false;
    bv_bool* temp_bv_bool_ptr = null;
    int data_includes = 0;
    bv_data* ptr_data_to_be_printed = null;
    size_t data_count_to_be_printed = 0;
    char* date_created_holder = malloc(100);
    char* date_last_modified_holder = malloc(100);
    char* parent_byte_id_holder = malloc(100);
    char* self_id_holder = malloc(100);
    char* name_holder = malloc(300);
    char* last_modifying_text_holder = malloc(300);
    bv_node* temp_node2 = null;
    bv_byte* ptr_byte = null;

    /*
     * 1 for data print and
     * 0 for info
     */
    if(flags == 0 || flags == 1) {
        //nothing
    } else {
        flags = 1; //default data print
    }
    if(flags == 1) {
        data_includes = 1;
    } else {
        data_includes = 0;
    }


    bv_print_byte_deeps(deep, deep_flags);
    (end_byte) ? (bv_print_char(BV_L_CHAR, 1)) : (bv_print_char(BV_PERPND_T_CHAR, 1));
    bv_print_char(BV_HIFEN_CHAR, BV_SYM_COUNT);
    bv_apply_byte_name_color();
    fprintf(stdout, "%s", byte->name);
    bv_console_text_color(0x3);
    fprintf(stdout, " %llu ", byte->id);
    bv_apply_byte_name_color();
    fprintf(stdout, "\n");
    bv_apply_normal_color();
    if(bv_logged_in_user != null) {
        rc = bv_get_direct_child_bytes(bv_logged_in_user->username, byte->id, data_includes, &node_ptr_temp, &errMsg);
        if(rc != BV_OP_SUCCESS) {
            node_ptr_temp = null;
        }
        if(node_ptr_temp == null) {
            child_bytes_count = 0;
        } else {
            child_bytes_count = bv_ll_count(node_ptr_temp);
        }
    } else {
        child_bytes_count = 0;
    }

    if(flags == 1) {
        ptr_data_to_be_printed = byte->data;
        data_count_to_be_printed = byte->data_count;
    } else {
        ptr_data_to_be_printed = malloc(6 * sizeof(bv_data));

        ptr_data_to_be_printed[0].key = "Name";
        strcpy(name_holder, byte->name);
        ptr_data_to_be_printed[0].value = name_holder;

        ptr_data_to_be_printed[1].key = "Date Created";
        bv_format_date(byte->date_created, date_created_holder);
        ptr_data_to_be_printed[1].value = date_created_holder;

        ptr_data_to_be_printed[2].key = "Last Modified Date";
        bv_format_date(byte->date_last_modified, date_last_modified_holder);
        ptr_data_to_be_printed[2].value = date_last_modified_holder;

        ptr_data_to_be_printed[3].key = "Id";
        sprintf(self_id_holder, "%llu", byte->id);
        ptr_data_to_be_printed[3].value = self_id_holder;

        ptr_data_to_be_printed[4].key = "Parent Byte Id";
        sprintf(parent_byte_id_holder, "%llu", byte->parent_byte_id);
        ptr_data_to_be_printed[4].value = parent_byte_id_holder;

        ptr_data_to_be_printed[5].key = "Last Modifying Text";
        strcpy(last_modifying_text_holder, byte->last_modifying_text);
        ptr_data_to_be_printed[5].value = last_modifying_text_holder;

        data_count_to_be_printed = 6;
    }

    if((data_count_to_be_printed + child_bytes_count) == 0) {
        bv_print_byte_deeps(deep, deep_flags);
        (end_byte) ? (bv_print_char(BV_SPACE_CHAR, 1)) : (bv_print_char(BV_PIPE_CHAR, 1));
        bv_print_char(BV_SPACE_CHAR, BV_SYM_COUNT);
        bv_print_char(BV_L_CHAR, 1);
        bv_print_char(BV_HIFEN_CHAR, BV_SYM_COUNT);
        bv_apply_byte_key_color();
        fprintf(stdout, "Empty\n");
        bv_apply_normal_color();
    } else {
        for(i = 0; i<(child_bytes_count + data_count_to_be_printed); i++) {
            if(i == ((child_bytes_count + data_count_to_be_printed) - 1)) {
                current_end_of_byte = bv_true;
            } else {
                current_end_of_byte = bv_false;
            }
            if(i < data_count_to_be_printed) {
                bv_print_byte_deeps(deep, deep_flags);
                (end_byte) ? (bv_print_char(BV_SPACE_CHAR, 1)) : (bv_print_char(BV_PIPE_CHAR, 1));
                bv_print_char(BV_SPACE_CHAR, BV_SYM_COUNT);

                if(current_end_of_byte)
                    bv_print_char(BV_L_CHAR, 1);
                else
                    bv_print_char(BV_PERPND_T_CHAR, 1);
                bv_print_char(BV_HIFEN_CHAR, BV_SYM_COUNT);
                bv_apply_byte_key_color();
                fprintf(stdout, "%s: ", ptr_data_to_be_printed[i].key);
                bv_apply_byte_value_color();
                fprintf(stdout, "%s\n", ptr_data_to_be_printed[i].value);
                bv_apply_normal_color();
            } else {
                temp_bv_bool_ptr = malloc(deep + 1);
                memcpy(temp_bv_bool_ptr, deep_flags, deep * sizeof(bv_bool));
                if(end_byte) {
                    temp_bv_bool_ptr[deep] = bv_false;
                } else {
                    temp_bv_bool_ptr[deep] = bv_true;
                }
                temp_byte_ptr = bv_ll_get(node_ptr_temp, i - data_count_to_be_printed)->value_ptr;
                bv_print_single_byte(current_end_of_byte, deep + 1, temp_bv_bool_ptr, temp_byte_ptr, flags);
            }
        }
    }

    while (node_ptr_temp != null){
        ptr_byte = node_ptr_temp->value_ptr;
        bv_free_byte_obj(ptr_byte);
        free(ptr_byte);
        temp_node2 = node_ptr_temp;
        node_ptr_temp = node_ptr_temp->next;
        free(temp_node2);
    }

    if(flags == 0 && ptr_data_to_be_printed != null) {
        free(ptr_data_to_be_printed);
    }

    if(deep_flags != null) {
        free(deep_flags);
    }
    free(date_created_holder);
    free(date_last_modified_holder);
    free(name_holder);
    free(last_modifying_text_holder);
    free(self_id_holder);
    free(parent_byte_id_holder);
}

static void bv_print_byte_deeps(size_t deep, bv_bool* deep_flags) {
    size_t i = 0;

    for(; i<deep; i++) {
        (deep_flags[i]) ? (bv_print_char(BV_PIPE_CHAR, 1)) : (bv_print_char(BV_SPACE_CHAR, 1));
        bv_print_char(BV_SPACE_CHAR, BV_SYM_COUNT);
    }
}


void bv_print_user_info(const char* title, bv_user* user, size_t byte_count, size_t bytes_data_count) {
    short int sym_count = BV_SYM_COUNT;

    fprintf(stdout, "%s\n", title);

    bv_print_char(BV_PERPND_T_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    fprintf(stdout, "Authentication\n");

    bv_print_char(BV_PIPE_CHAR, 1);
    bv_print_char(BV_SPACE_CHAR, sym_count);
    bv_print_char(BV_PERPND_T_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    bv_apply_byte_key_color();
    fprintf(stdout, "Username: ");
    bv_apply_byte_value_color();
    fprintf(stdout, "%s\n", user->username);
    bv_apply_normal_color();

    bv_print_char(BV_PIPE_CHAR, 1);
    bv_print_char(BV_SPACE_CHAR, sym_count);
    bv_print_char(BV_PERPND_T_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    bv_apply_byte_key_color();
    fprintf(stdout, "Password: ");
    bv_apply_byte_value_color();
    fprintf(stdout, "%s\n", user->password);
    bv_apply_normal_color();

    bv_print_char(BV_PIPE_CHAR, 1);
    bv_print_char(BV_SPACE_CHAR, sym_count);
    bv_print_char(BV_PERPND_T_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    bv_apply_byte_key_color();
    fprintf(stdout, "Hints: ");
    bv_apply_byte_value_color();
    fprintf(stdout, "%s\n", user->hints);
    bv_apply_normal_color();

    bv_print_char(BV_PIPE_CHAR, 1);
    bv_print_char(BV_SPACE_CHAR, sym_count);
    bv_print_char(BV_PERPND_T_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    bv_apply_byte_key_color();
    fprintf(stdout, "Security Question: ");
    bv_apply_byte_value_color();
    fprintf(stdout, "%s\n", user->sec_ques);
    bv_apply_normal_color();

    bv_print_char(BV_PIPE_CHAR, 1);
    bv_print_char(BV_SPACE_CHAR, sym_count);
    bv_print_char(BV_L_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    bv_apply_byte_key_color();
    fprintf(stdout, "Security Question Answer: ");
    bv_apply_byte_value_color();
    fprintf(stdout, "%s\n", user->sec_ques_ans);
    bv_apply_normal_color();


    bv_print_char(BV_L_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    fprintf(stdout, "Others\n");

    bv_print_char(BV_SPACE_CHAR, sym_count + 1);
    bv_print_char(BV_PERPND_T_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    bv_apply_byte_key_color();
    fprintf(stdout, "Encryption Key: ");
    bv_apply_byte_value_color();
    fprintf(stdout, "%s\n", user->enc_key);
    bv_apply_normal_color();

    bv_print_char(BV_SPACE_CHAR, sym_count + 1);
    bv_print_char(BV_PERPND_T_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    bv_apply_byte_key_color();
    fprintf(stdout, "Bytes count: ");
    bv_apply_byte_value_color();
    fprintf(stdout, "%llu\n", byte_count);
    bv_apply_normal_color();

    bv_print_char(BV_SPACE_CHAR, sym_count + 1);
    bv_print_char(BV_L_CHAR, 1);
    bv_print_char(BV_HIFEN_CHAR, sym_count);
    bv_apply_byte_key_color();
    fprintf(stdout, "Bytes Data count: ");
    bv_apply_byte_value_color();
    fprintf(stdout, "%llu\n", bytes_data_count);
    bv_apply_normal_color();
}

void bv_console_text_color(int color_code) {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, (WORD)color_code);
}

void bv_apply_input_string_key_color() {
    bv_console_text_color(BV_INPUT_STRING_KEY_COLOR);
}

void bv_apply_input_string_value_color() {
    bv_console_text_color(BV_INPUT_STRING_VALUE_COLOR);
}

void bv_apply_byte_key_color() {
    bv_console_text_color(BV_BYTE_KEY_COLOR);
}

void bv_apply_byte_value_color() {
    bv_console_text_color(BV_BYTE_VALUE_COLOR);
}

void bv_apply_normal_color() {
    bv_console_text_color(BV_NORMAL_COLOR);
}


void bv_print_char(int chr, int count) {
    int i = 0;
    for(i=0; i<count; i++) {
        fprintf(stdout, "%c", chr);
    }
}

void bv_apply_byte_name_color() {
    bv_console_text_color(BV_BYTE_NAME_COLOR);
}

void bv_format_date(time_t time_in_secs, char* dest) {
    struct tm* local_time_tm_ptr = null;
    size_t ln;

    local_time_tm_ptr = localtime(&time_in_secs);
    sprintf(dest, "%s", asctime(local_time_tm_ptr));
    ln = strlen(dest);
   (ln >= 1 && dest[ln - 1] == '\n') ? (dest[ln - 1] = '\0') : (0);
}





























