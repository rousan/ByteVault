//
// Created by Rousan Ali on 05-10-2016.
//

#include <windows.h>

#include "include/database.h"
#include "include/utils.h"
#include "include/sqlite/sqlite3.h"
#include "include/constants.h"
#include "include/logger.h"
#include "include/user.h"
#include "include/encryption.h"
#include "include/error_messages.h"

bv_user* bv_logged_in_user;

static sqlite3* users_db_connection;
static bv_node* users_data_db_connection;

static void* bv_select_op_data_ptr;
static size_t bv_select_op_data_call_count;
static void bv_init_select_op_params();
static int bv_select_data_callback(void *op_code, int argc, char **argv, char **azColName);
static int bv_compare_function_ull(const void * a, const void * b);
static int bv_backup_single_byte(FILE* output_stream, const char* username, size_t byte_id, bv_bool recursive, char** errMsg);
static int bv_delete_single_byte(const char* username, size_t byte_id, char** errMsg);
static int bv_delete_byte_with_recursive(const char *username, size_t byte_id, char **errMsg);
static int bv_delete_byte_without_recursive(const char* username, size_t byte_id, char** errMsg);



void bv_get_users_db_connection(sqlite3** connection) {
    if (users_db_connection == NULL) {
        char* temp = malloc(100);
        int rc;
        char* sql = NULL;
        char* errorMsg = NULL;

        bv_get_root_folder_path(temp, 100);
        strcat(temp, "\\");
        strcat(temp, (BV_FOLDER_DATA));
        strcat(temp, "\\");
        strcat(temp, BV_FILE_USERS_DB);
        rc = sqlite3_open(temp, &users_db_connection);
        if (rc != SQLITE_OK) {
            if(rc == SQLITE_NOTADB) {
                if(users_db_connection != NULL) {
                    sqlite3_close(users_db_connection);
                }
                remove(temp);
                rc = sqlite3_open(temp, &users_db_connection);
                if(rc != SQLITE_OK) {
                    bv_error("Can't open database");
                    bv_exit(-1);
                }
            } else {
                bv_error("Can't open database");
                bv_exit(-1);
            }
        }
        sql = malloc(500);
        sprintf(sql, "create table if not exists %s "\
                "( "\
                "%s text not null, "\
                "%s text not null, "\
                "%s text not null, "\
                "%s text not null, "\
                "%s text not null, "\
                "%s text not null, "\
                "%s integer primary key"\
                ")",
                BV_USERS_DB_TABLE_NAME_USERS_INFO,
                BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_USERNAME,
                BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_PASSWORD,
                BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_HINTS,
                BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_ENCRYPTION_KEY,
                BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES,
                BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES_ANS,
                BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_ID
        );
        bv_init_select_op_params();
        rc = sqlite3_exec(users_db_connection, sql, NULL, NULL, &errorMsg);
        if(rc != SQLITE_OK) {
            if(rc == SQLITE_NOTADB) {
                if(users_db_connection != NULL) {
                    sqlite3_close(users_db_connection);
                }
                remove(temp);
                rc = sqlite3_open(temp, &users_db_connection);
                if(rc != SQLITE_OK) {
                    bv_error("Can't open database");
                    bv_exit(-1);
                }
                bv_init_select_op_params();
                rc = sqlite3_exec(users_db_connection, sql, NULL, NULL, &errorMsg);
                if(rc != SQLITE_OK) {
                    bv_error(errorMsg);
                    sqlite3_free(errorMsg);
                    bv_exit(-1);
                }
            } else {
                bv_error(errorMsg);
                sqlite3_free(errorMsg);
                bv_exit(-1);
            }
        }
        free(temp);
        free(sql);
    }
    *connection = users_db_connection;
}

void bv_get_user_data_db_connection(sqlite3 **connection, const char *username) {
    char* usrname_temp_ptr = null;
    bv_user_data_db_conn_struct* ptr_bv_user_data_db_conn_struct_temp = null;
    bv_node* node_ptr_temp = null;
    bv_bool conn_exist;

    conn_exist = bv_false; //must
    if(users_data_db_connection == null) {
        conn_exist = bv_false;
    } else {
        node_ptr_temp = users_data_db_connection;
        while (node_ptr_temp != null) {
            ptr_bv_user_data_db_conn_struct_temp = node_ptr_temp->value_ptr;
            if(!strcmp(ptr_bv_user_data_db_conn_struct_temp->username, username)) {
                conn_exist = bv_true;
                break;
            }
            node_ptr_temp = node_ptr_temp->next;
        }
    }

    if(conn_exist) {
        ptr_bv_user_data_db_conn_struct_temp = node_ptr_temp->value_ptr;
    } else {
        usrname_temp_ptr = malloc(strlen(username) + 10);
        strcpy(usrname_temp_ptr, username);
        ptr_bv_user_data_db_conn_struct_temp = malloc(sizeof(bv_user_data_db_conn_struct));
        ptr_bv_user_data_db_conn_struct_temp->username = usrname_temp_ptr;
        ptr_bv_user_data_db_conn_struct_temp->db_conn = null;
        node_ptr_temp = malloc(sizeof(bv_node));
        node_ptr_temp->value_ptr = ptr_bv_user_data_db_conn_struct_temp;
        node_ptr_temp->next = null; //must
        if(users_data_db_connection == null) {
            users_data_db_connection = node_ptr_temp;
            users_data_db_connection->next = null; //must
        } else {
            bv_ll_add_node(users_data_db_connection, node_ptr_temp);
        }
    }

    if (ptr_bv_user_data_db_conn_struct_temp->db_conn == NULL) {
        char* temp = malloc(100);
        int rc;
        char* sql = NULL;
        char* errorMsg = NULL;

        bv_get_root_folder_path(temp, 100);
        strcat(temp, "\\");
        strcat(temp, (BV_FOLDER_DATA));
        strcat(temp, "\\");
        strcat(temp, username);
        strcat(temp, BV_APPEND_STRING_USERS_DATA_DB);
        rc = sqlite3_open(temp, &ptr_bv_user_data_db_conn_struct_temp->db_conn);
        if (rc != SQLITE_OK) {
            if(rc == SQLITE_NOTADB) {
                if(ptr_bv_user_data_db_conn_struct_temp->db_conn != NULL) {
                    sqlite3_close(ptr_bv_user_data_db_conn_struct_temp->db_conn);
                }
                remove(temp);
                rc = sqlite3_open(temp, &ptr_bv_user_data_db_conn_struct_temp->db_conn);
                if(rc != SQLITE_OK) {
                    bv_error("Can't open database");
                    bv_exit(-1);
                }
            } else {
                bv_error("Can't open database");
                bv_exit(-1);
            }
        }
        sql = malloc(500);
        sprintf(sql, "create table if not exists %s ("
                        "%s text not null,"
                        "%s text not null,"
                        "%s text not null,"
                        "%s text not null,"
                        "%s text not null,"
                        "%s integer primary key"
                        ")",
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_NAME,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_PARENT_BYTE_ID,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_CREATED,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID
        );
        bv_init_select_op_params();
        rc = sqlite3_exec(ptr_bv_user_data_db_conn_struct_temp->db_conn, sql, NULL, NULL, &errorMsg);
        if(rc != SQLITE_OK) {
            if(rc == SQLITE_NOTADB) {
                if(ptr_bv_user_data_db_conn_struct_temp->db_conn != NULL) {
                    sqlite3_close(ptr_bv_user_data_db_conn_struct_temp->db_conn);
                }
                remove(temp);
                rc = sqlite3_open(temp, &ptr_bv_user_data_db_conn_struct_temp->db_conn);
                if(rc != SQLITE_OK) {
                    bv_error("Can't open database");
                    bv_exit(-1);
                }
                bv_init_select_op_params();
                rc = sqlite3_exec(ptr_bv_user_data_db_conn_struct_temp->db_conn, sql, NULL, NULL, &errorMsg);
                if(rc != SQLITE_OK) {
                    bv_error(errorMsg);
                    sqlite3_free(errorMsg);
                    bv_exit(-1);
                }
            } else {
                bv_error(errorMsg);
                sqlite3_free(errorMsg);
                bv_exit(-1);
            }
        }

        sprintf(sql, "create table if not exists %s ("
                        "%s text not null,"
                        "%s text not null,"
                        "%s text not null,"
                        "%s integer primary key"
                        ")",
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_KEY,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_VALUE,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_BYTE_ID,
                BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_ID
        );
        bv_init_select_op_params();
        rc = sqlite3_exec(ptr_bv_user_data_db_conn_struct_temp->db_conn, sql, NULL, NULL, &errorMsg);
        if(rc != SQLITE_OK) {
            if(rc == SQLITE_NOTADB) {
                if(ptr_bv_user_data_db_conn_struct_temp->db_conn != NULL) {
                    sqlite3_close(ptr_bv_user_data_db_conn_struct_temp->db_conn);
                }
                remove(temp);
                rc = sqlite3_open(temp, &ptr_bv_user_data_db_conn_struct_temp->db_conn);
                if(rc != SQLITE_OK) {
                    bv_error("Can't open database");
                    bv_exit(-1);
                }
                bv_init_select_op_params();
                rc = sqlite3_exec(ptr_bv_user_data_db_conn_struct_temp->db_conn, sql, NULL, NULL, &errorMsg);
                if(rc != SQLITE_OK) {
                    bv_error(errorMsg);
                    sqlite3_free(errorMsg);
                    bv_exit(-1);
                }
            } else {
                bv_error(errorMsg);
                sqlite3_free(errorMsg);
                bv_exit(-1);
            }
        }
        free(temp);
        free(sql);
    }
    *connection = ptr_bv_user_data_db_conn_struct_temp->db_conn;
}

void bv_open_db_connections() {
    sqlite3* temp_connection = NULL;
    bv_get_users_db_connection(&temp_connection);
    if(bv_logged_in_user != NULL && bv_logged_in_user->username != NULL) {
        bv_get_user_data_db_connection(&temp_connection, bv_logged_in_user->username);
    }
}

void bv_close_db_connections() {
    bv_node* temp_node_ptr = null;
    bv_node* temp_node_ptr2 = null;
    bv_user_data_db_conn_struct* bv_user_data_db_conn_struct_ptr = null;

    if(users_db_connection != NULL) {
        sqlite3_close(users_db_connection);
    }
    temp_node_ptr = users_data_db_connection;
    while(temp_node_ptr != null) {
        bv_user_data_db_conn_struct_ptr = temp_node_ptr->value_ptr;
        if(bv_user_data_db_conn_struct_ptr->username != null) {
            free((char*)bv_user_data_db_conn_struct_ptr->username);
        }
        if(bv_user_data_db_conn_struct_ptr->db_conn != null) {
            sqlite3_close(bv_user_data_db_conn_struct_ptr->db_conn);
        }
        free(bv_user_data_db_conn_struct_ptr);
        temp_node_ptr2 = temp_node_ptr->next;
        free(temp_node_ptr);
        temp_node_ptr = temp_node_ptr2;
    }

    users_db_connection = null;
    users_data_db_connection = null;
}


void bv_escape_sqlite_value(char* dest, const char* value) {
    strcpy(dest, "");
    char* found_ptr = NULL;
    while ( value[0] != '\0' && (found_ptr = strchr(value, '\'')) != NULL) {
        strncat(dest, value, found_ptr - value);
        strcat(dest, "''");
        value = found_ptr + 1;
    }
    strcat(dest, value);
}

BV_DEPRECATED void bv_escape_sqlite_value_for_like_clause(char* dest, const char* value) {
    char* temp = malloc(strlen(value) * 2 + 10);
    char* found_ptr = NULL;
    char* temp_char_p = NULL;
    char* temp_ptr = NULL;
    char chars_to_escape[] = {'%', '_', BV_SQLITE_ESCAPE_CHAR[0]};
    int i = 0;

    strcpy(dest, "");
    bv_escape_sqlite_value(temp, value);

    temp_char_p = temp;
    while ( temp_char_p[0] != '\0') {
        found_ptr = NULL;
        for(i=0; i<((int)sizeof(chars_to_escape)); i++) {
            temp_ptr = strchr(temp_char_p, chars_to_escape[i]);
            if(found_ptr == NULL) {
                found_ptr = temp_ptr;
            } else {
                if(temp_ptr != NULL && temp_ptr < found_ptr) {
                    found_ptr = temp_ptr;
                }
            }
        }
        if(found_ptr == NULL) {
            break;
        }
        strncat(dest, temp_char_p, found_ptr - temp_char_p);
        strcat(dest, BV_SQLITE_ESCAPE_CHAR);
        strncat(dest, found_ptr, 1);
        temp_char_p = found_ptr + 1;
    }
    strcat(dest, temp_char_p);
}


int bv_insert_user(bv_user* user, char** errorMsg) {
    sqlite3* users_db_connection = NULL;
    char* errormsg = NULL;
    char* sql;
    int rc;
    char* username = malloc(50);
    char* password = malloc(50);
    char* hints = malloc(220);
    char* enc_key = malloc(100);
    char* sec_ques = malloc(110);
    char* sec_ques_ans = malloc(110);
    char* temp = malloc(110);

    bv_encrypt_text(user->username, BV_PRIMARY_KEY, temp);
    bv_escape_sqlite_value(username, temp);
    bv_encrypt_text(user->enc_key, BV_PRIMARY_KEY, temp);
    bv_escape_sqlite_value(enc_key, temp);

    bv_encrypt_text(user->password, user->enc_key, temp);
    bv_escape_sqlite_value(password, temp);
    bv_encrypt_text(user->hints, user->enc_key, temp);
    bv_escape_sqlite_value(hints, temp);
    bv_encrypt_text(user->sec_ques, user->enc_key, temp);
    bv_escape_sqlite_value(sec_ques, temp);
    bv_encrypt_text(user->sec_ques_ans, user->enc_key, temp);
    bv_escape_sqlite_value(sec_ques_ans, temp);

    bv_get_users_db_connection(&users_db_connection);
    sql = malloc(500);
    sprintf(sql, "insert into %s (%s, %s, %s, %s, %s, %s) values ('%s', '%s', '%s', '%s', '%s', '%s')",
            BV_USERS_DB_TABLE_NAME_USERS_INFO,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_USERNAME,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_PASSWORD,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_ENCRYPTION_KEY,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_HINTS,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES_ANS,
            username, password, enc_key, hints, sec_ques, sec_ques_ans
    );
    bv_init_select_op_params();
    rc = sqlite3_exec(users_db_connection, sql, NULL, NULL, &errormsg);
    free(username);
    free(password);
    free(enc_key);
    free(hints);
    free(sec_ques);
    free(sec_ques_ans);
    free(temp);
    free(sql);
    if(rc != SQLITE_OK) {
        *errorMsg = errormsg;
        return BV_OP_FAILED;
    } else {
        return BV_OP_SUCCESS;
    }
}

static int bv_select_data_callback(void *op_code, int argc, char **argv, char **azColName) {
    bv_select_op_code* pOpCode = op_code;
    if(pOpCode->op_code == 1 && bv_select_op_data_call_count == 0) {
        size_t i = 0;
        char* usernm = malloc(30);
        char* password = malloc(30);
        char* hints = malloc(110);
        char* sec_ques = malloc(110);
        char* sec_ques_ans = malloc(110);
        char* enc_key = malloc(50);
        bv_user* user = malloc(sizeof(bv_user));
        for(; i<argc; i++) {
            if (!strcmpi(azColName[i], BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_USERNAME)) {
                strcpy(usernm, argv[i]);
                user->username = usernm;
            } else if (!strcmpi(azColName[i],BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_PASSWORD)) {
                strcpy(password, argv[i]);
                user->password = password;
            } else if (!strcmpi(azColName[i], BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_HINTS)) {
                strcpy(hints, argv[i]);
                user->hints = hints;
            } else if (!strcmpi(azColName[i], BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_ENCRYPTION_KEY)) {
                strcpy(enc_key, argv[i]);
                user->enc_key = enc_key;
            } else if (!strcmpi(azColName[i], BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES)) {
                strcpy(sec_ques, argv[i]);
                user->sec_ques = sec_ques;
            } else if (!strcmpi(azColName[i], BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES_ANS)) {
                strcpy(sec_ques_ans, argv[i]);
                user->sec_ques_ans = sec_ques_ans;
            }
        }
        bv_select_op_data_call_count++;
        bv_select_op_data_ptr = user;
    } else if(pOpCode->op_code == 2) {
        size_t i = 0;
        char* name = null;
        char* last_modifying_text = null;
        char* date_created = malloc(50);
        char* date_last_modified = malloc(50);
        char* parent_byte_id = malloc(50);
        char* id = malloc(50);
        bv_byte* byte = malloc(sizeof(bv_byte));
        bv_node* node = malloc(sizeof(bv_node));
        char* temp = malloc(300);
        byte->data = NULL;
        byte->data_count = 0;
        node->value_ptr = byte;
        if(bv_select_op_data_call_count == 0) {
            bv_select_op_data_ptr = node;
            node->next = NULL;
        } else {
            bv_ll_add_node((bv_node*)bv_select_op_data_ptr, node);
        }
        for(; i<argc; i++) {
            if(!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_NAME)) {
                strcpy(temp, argv[i]);
                name = malloc(strlen(temp) + 10);
                bv_decrypt_text(temp, pOpCode->extra_data, name);
                byte->name = name;
            } else if (!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT)) {
                strcpy(temp, argv[i]);
                last_modifying_text = malloc(strlen(temp) + 10);
                bv_decrypt_text(temp, pOpCode->extra_data, last_modifying_text);
                byte->last_modifying_text = last_modifying_text;
            } else if (!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_CREATED)) {
                strcpy(temp, argv[i]);
                bv_decrypt_text(temp, pOpCode->extra_data, date_created);
                byte->date_created = atoll(date_created);
            } else if (!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED)) {
                strcpy(temp, argv[i]);
                bv_decrypt_text(temp, pOpCode->extra_data, date_last_modified);
                byte->date_last_modified = atoll(date_last_modified);
            } else if (!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_PARENT_BYTE_ID)) {
                strcpy(temp, argv[i]);
                bv_decrypt_text(temp, pOpCode->extra_data, parent_byte_id);
                byte->parent_byte_id = (unsigned long long)atoll(parent_byte_id);
            } else if (!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID)) {
                //Id is not encrypted
                strcpy(temp, argv[i]);
                strcpy(id, temp);
                byte->id = (unsigned long long)atoll(id);
            }
        }
        bv_select_op_data_call_count++;
        free(temp);
        free(date_created);
        free(date_last_modified);
        free(id);
        free(parent_byte_id);
    } else if(pOpCode->op_code == 3) {
        size_t i = 0;
        char* key = null;
        char* value = null;
        char* temp = malloc(BV_BYTE_DATA_VALUE_LENGTH + 200);
        bv_data* temp_data_ptr = NULL;
        if(bv_select_op_data_call_count == 0) {
            bv_select_op_data_ptr = malloc(sizeof(bv_data));
            temp_data_ptr = bv_select_op_data_ptr;
        } else {
            bv_select_op_data_ptr = realloc(bv_select_op_data_ptr, (sizeof(bv_data) * (bv_select_op_data_call_count + 1)));
            temp_data_ptr = (bv_data*)bv_select_op_data_ptr + bv_select_op_data_call_count;
        }
        for(; i<argc; i++) {
            if (!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_KEY)) {
                strcpy(temp, argv[i]);
                key = malloc(strlen(temp) + 10);
                bv_decrypt_text(temp, pOpCode->extra_data, key);
                temp_data_ptr->key = key;
            } else if (!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_VALUE)) {
                strcpy(temp, argv[i]);
                value = malloc(strlen(temp) + 10);
                bv_decrypt_text(temp, pOpCode->extra_data, value);
                temp_data_ptr->value = value;
            }
        }
        bv_select_op_data_call_count++;
        free(temp);
    } else if(pOpCode->op_code == 4) {
        size_t i = 0;
        unsigned long long* id = malloc(sizeof(unsigned long long));
        *id = 0;
        char* temp = malloc(300);
        bv_node* node = malloc(sizeof(bv_node));
        node->next = null;
        node->value_ptr = id;
        if(bv_select_op_data_call_count == 0) {
            bv_select_op_data_ptr = node;
            node->next = NULL;
        } else {
            bv_ll_add_node((bv_node*)bv_select_op_data_ptr, node);
        }

        for(; i<argc; i++) {
            if (!strcmpi(azColName[i], BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID)) {
                strcpy(temp, argv[i]);
                *id = (unsigned long long)atoll(temp);
            }
        }
        bv_select_op_data_call_count++;
        free(temp);
    }
    return 0;
}

static void bv_init_select_op_params() {
    bv_select_op_data_call_count = 0;
    bv_select_op_data_ptr = NULL;
}

int bv_get_user_info(const char* username, bv_user* user, char** errMsg) {
    sqlite3* users_db_connection = NULL;
    char* errormsg = NULL;
    char* sql;
    int rc;
    char* username_temp = malloc(50);
    char* temp = malloc(30);
    bv_select_op_code op_code = {1};
    bv_user* user_out = NULL;


    bv_encrypt_text(username, BV_PRIMARY_KEY, temp);
    bv_escape_sqlite_value(username_temp, temp);
    bv_get_users_db_connection(&users_db_connection);
    sql = malloc(500);
    sprintf(sql, "select * from %s where %s='%s' limit 1",
            BV_USERS_DB_TABLE_NAME_USERS_INFO,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_USERNAME,
            username_temp
    );
    bv_init_select_op_params();//It must to call before any query operation.
    rc = sqlite3_exec(users_db_connection, sql, bv_select_data_callback, &op_code, &errormsg);
    free(username_temp);
    free(temp);
    free(sql);
    if(rc != SQLITE_OK) {
        *errMsg = errormsg;
        return BV_OP_FAILED;
    }
    user_out = bv_select_op_data_ptr;
    if(user_out == NULL && bv_select_op_data_call_count == 0) {
        *errMsg = "The username does not exist";
        return BV_OP_FAILED;
    } else if(user_out == NULL) {
        *errMsg = BV_ERROR_MSG_UNKNOWN_ERROR;
        return BV_OP_FAILED;
    }
    char* usernm = malloc(strlen(user_out->username) + 10);
    char* password = malloc(strlen(user_out->password) + 10);
    char* hints = malloc(strlen(user_out->hints) + 10);
    char* enc_key = malloc(strlen(user_out->enc_key) + 10);
    char* sec_ques = malloc(strlen(user_out->sec_ques) + 10);
    char* sec_ques_ans = malloc(strlen(user_out->sec_ques_ans) + 10);
    bv_decrypt_text(user_out->username, BV_PRIMARY_KEY, usernm);
    bv_decrypt_text(user_out->enc_key, BV_PRIMARY_KEY, enc_key);
    bv_decrypt_text(user_out->password, enc_key, password);
    bv_decrypt_text(user_out->hints, enc_key, hints);
    bv_decrypt_text(user_out->sec_ques, enc_key, sec_ques);
    bv_decrypt_text(user_out->sec_ques_ans, enc_key, sec_ques_ans);
    user->username = usernm;
    user->password = password;
    user->hints = hints;
    user->enc_key = enc_key;
    user->sec_ques = sec_ques;
    user->sec_ques_ans = sec_ques_ans;
    free((char*)user_out->username);
    free((char*)user_out->password);
    free((char*)user_out->hints);
    free((char*)user_out->enc_key);
    free((char*)user_out->sec_ques);
    free((char*)user_out->sec_ques_ans);
    free(user_out);
    return BV_OP_SUCCESS;
}


int bv_get_direct_child_bytes(const char* username, size_t byte_id, int is_byte_data_includes, bv_node** head_node, char** errMsg) {
    sqlite3* user_data_db_connection;
    int rc;
    char* errMessage;
    char* sql = NULL;
    bv_user temp_user;
    char* temp = NULL;
    char* temp2 = NULL;
    char* byte_id_holder = null;
    bv_select_op_code op_code;
    bv_node* temp_node_ptr = NULL;
    bv_node* temp_node_ptr2= NULL;
    bv_node* temp_node_ptr3 = NULL;
    bv_node* temp_node_ptr4 = NULL;


    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        return BV_OP_FAILED;
    }


    temp2 = malloc(300);
    temp = malloc(300);
    sql = malloc(500);
    byte_id_holder = malloc(120);

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(sql);
        free(temp);
        free(temp2);
        free(byte_id_holder);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(sql);
        free(temp);
        free(temp2);
        free(byte_id_holder);
        return BV_OP_FAILED;
    }

    sprintf(temp2, "%llu", byte_id);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(byte_id_holder, temp);


    bv_get_user_data_db_connection(&user_data_db_connection, username);
    sprintf(sql, "select %s from %s where %s='%s'",
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_PARENT_BYTE_ID,
            byte_id_holder
    );
    op_code.op_code = 4;
    op_code.extra_data = (void*)temp_user.enc_key;
    bv_init_select_op_params();
    rc = sqlite3_exec(user_data_db_connection, sql, bv_select_data_callback, (void*)&op_code, &errMessage);
    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        free(sql);
        free(temp);
        free(temp2);
        free(byte_id_holder);
        return BV_OP_FAILED;
    }

    temp_node_ptr = bv_select_op_data_ptr;
    if(temp_node_ptr == NULL && bv_select_op_data_call_count == 0) {
        *head_node = NULL;
        free(sql);
        free(temp);
        free(temp2);
        free(byte_id_holder);
        return BV_OP_SUCCESS;
    } else if(temp_node_ptr == NULL) {
        *errMsg = BV_ERROR_MSG_UNKNOWN_ERROR;
        free(sql);
        free(temp);
        free(temp2);
        free(byte_id_holder);
        return BV_OP_FAILED;
    }

    temp_node_ptr3 = null;
    while( temp_node_ptr != null ) {
        sprintf(temp, "-i %llu", *((unsigned long long*)temp_node_ptr->value_ptr));
        rc = bv_query_bytes(username, temp, 1, null, is_byte_data_includes, &temp_node_ptr2, &errMessage);
        if(rc != BV_OP_SUCCESS) {
            *errMsg = errMessage;
            free(sql);
            free(temp);
            free(temp2);
            free(byte_id_holder);
            temp_node_ptr4 = temp_node_ptr;
            if(temp_node_ptr4->value_ptr != null) {
                free(temp_node_ptr4->value_ptr);
            }
            free(temp_node_ptr4);
            return BV_OP_FAILED;
        }
        if(temp_node_ptr2 != null) {
            if(temp_node_ptr3 == null) {
                temp_node_ptr3 = temp_node_ptr2;
            } else {
                bv_ll_add_node(temp_node_ptr3, temp_node_ptr2);
            }
        }
        temp_node_ptr4 = temp_node_ptr;
        temp_node_ptr = temp_node_ptr->next;
        if(temp_node_ptr4->value_ptr != null) {
            free(temp_node_ptr4->value_ptr);
        }
        free(temp_node_ptr4);
    }


    *head_node = temp_node_ptr3;
    free(sql);
    free(temp);
    free(temp2);
    free(byte_id_holder);

    bv_free_user_obj(&temp_user);
    return BV_OP_SUCCESS;
}


int bv_insert_byte(const char* username, bv_byte* byte, char** errMsg) {
    sqlite3* user_data_db_connection;
    int rc;
    char* errMessage;
    char* sql = NULL;
    bv_user temp_user;
    char* temp = NULL;
    char* temp2 = NULL;
    char* name_temp = NULL;
    char* last_modifying_text = null;
    char* date_created_temp = NULL;
    char* parent_byte_id_temp = NULL;
    char* date_last_modified_temp = NULL;
    bv_node* temp_node_ptr = NULL;

    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        return BV_OP_FAILED;
    }

    name_temp = malloc(210);
    last_modifying_text = malloc(210);
    date_created_temp = malloc(50);
    parent_byte_id_temp = malloc(50);
    date_last_modified_temp = malloc(50);
    temp2 = malloc(100);
    temp = malloc(300);

    if(byte->parent_byte_id != 0) {
        sprintf(temp, "-i %llu", byte->parent_byte_id);
        rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
        if(rc == BV_OP_FAILED) {
            *errMsg = errMessage;
            free(name_temp);
            free(last_modifying_text);
            free(date_created_temp);
            free(parent_byte_id_temp);
            free(date_last_modified_temp);;
            free(temp);
            free(temp2);
            return BV_OP_FAILED;
        }
        if(temp_node_ptr == NULL) {
            *errMsg = "This byte's parent byte does not exist";
            free(name_temp);
            free(last_modifying_text);
            free(date_created_temp);
            free(parent_byte_id_temp);
            free(date_last_modified_temp);;
            free(temp);
            free(temp2);
            return BV_OP_FAILED;
        }
    }

    sprintf(temp2, "%llu", byte->date_created);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(date_created_temp, temp);
    sprintf(temp2, "%llu", byte->parent_byte_id);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(parent_byte_id_temp, temp);
    sprintf(temp2, "%llu", byte->date_last_modified);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(date_last_modified_temp, temp);
    bv_encrypt_text(byte->name, temp_user.enc_key, temp);
    bv_escape_sqlite_value(name_temp, temp);
    bv_encrypt_text(byte->last_modifying_text, temp_user.enc_key, temp);
    bv_escape_sqlite_value(last_modifying_text, temp);

    sql = malloc(750);
    bv_get_user_data_db_connection(&user_data_db_connection, username);
    sprintf(sql, "insert into %s (%s, %s, %s, %s, %s) values ('%s', '%s', '%s', '%s', '%s')",
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_NAME,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_CREATED,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_PARENT_BYTE_ID,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED,
            name_temp, last_modifying_text, date_created_temp, parent_byte_id_temp, date_last_modified_temp
    );
    bv_init_select_op_params();
    rc = sqlite3_exec(user_data_db_connection, sql, NULL, NULL, &errMessage);
    if(rc != SQLITE_OK) {
        free(sql);
        free(name_temp);
        free(last_modifying_text);
        free(date_created_temp);
        free(parent_byte_id_temp);
        free(date_last_modified_temp);;
        free(temp);
        free(temp2);
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }

    sprintf(temp, "order by %s desc", BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID);
    rc = bv_query_bytes(username, "all", 1, temp, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        free(sql);
        free(name_temp);
        free(last_modifying_text);
        free(date_created_temp);
        free(parent_byte_id_temp);
        free(date_last_modified_temp);;
        free(temp);
        free(temp2);
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        free(sql);
        free(name_temp);
        free(last_modifying_text);
        free(date_created_temp);
        free(parent_byte_id_temp);
        free(date_last_modified_temp);;
        free(temp);
        free(temp2);
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }

    free(name_temp);
    free(last_modifying_text);
    free(date_created_temp);
    free(parent_byte_id_temp);
    free(date_last_modified_temp);
    free(sql);
    free(temp);
    free(temp2);
    bv_free_user_obj(&temp_user);

    byte->id = ((bv_byte*)temp_node_ptr->value_ptr)->id;

    return BV_OP_SUCCESS;
}

int bv_insert_byte_data(const char* username, size_t byte_id, bv_data* data, size_t data_count, char** errMsg) {
    sqlite3* user_data_db_connection;
    char* temp = malloc(BV_BYTE_DATA_VALUE_LENGTH * 2 + 100);
    char* temp2 = malloc(200);
    bv_user temp_user;
    char* key_temp = NULL;
    char* value_temp = NULL;
    char* byte_id_temp = NULL;
    int rc;
    char* errMessage;
    bv_node* temp_node_ptr = NULL;
    char* sql = NULL;
    size_t i = 0;
    int temp_int = 0;
    size_t j = 0;
    size_t count = 0;

    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        free(temp2);
        return BV_OP_FAILED;
    }

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(temp);
        free(temp2);
        return BV_OP_FAILED;
    }
    sql = malloc(BV_BYTE_DATA_VALUE_LENGTH * 2 + 400);
    bv_get_user_data_db_connection(&user_data_db_connection, username);
    key_temp = malloc(300);
    value_temp = malloc(BV_BYTE_DATA_VALUE_LENGTH * 2 + 100);
    byte_id_temp = malloc(100);

    for(i=0; i<data_count; i++) {
        temp_int = 0;
        for(j=0; j<((bv_byte*)temp_node_ptr->value_ptr)->data_count; j++) {
            if(!strcmp(data[i].key, ((bv_byte*)temp_node_ptr->value_ptr)->data[j].key)) {
                temp_int = 1;
                break;
            }
        };
        if(temp_int == 0) {
            bv_encrypt_text(data[i].key, temp_user.enc_key, temp);
            bv_escape_sqlite_value(key_temp, temp);
            bv_encrypt_text(data[i].value, temp_user.enc_key, temp);
            bv_escape_sqlite_value(value_temp, temp);
            sprintf(temp2, "%llu", byte_id);
            bv_encrypt_text(temp2, temp_user.enc_key, temp);
            bv_escape_sqlite_value(byte_id_temp, temp);
            sprintf(sql, "insert into %s (%s, %s, %s) values ('%s', '%s', '%s')",
                    BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA,
                    BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_KEY,
                    BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_VALUE,
                    BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_BYTE_ID,
                    key_temp, value_temp, byte_id_temp
            );
        } else {
            bv_encrypt_text(data[i].key, temp_user.enc_key, temp);
            bv_escape_sqlite_value(key_temp, temp);
            bv_encrypt_text(data[i].value, temp_user.enc_key, temp);
            bv_escape_sqlite_value(value_temp, temp);
            sprintf(temp2, "%llu", byte_id);
            bv_encrypt_text(temp2, temp_user.enc_key, temp);
            bv_escape_sqlite_value(byte_id_temp, temp);
            sprintf(sql, "update %s set %s='%s' where %s='%s' and %s='%s'",
                    BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA,
                    BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_VALUE,
                    value_temp,
                    BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_KEY,
                    key_temp,
                    BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_BYTE_ID,
                    byte_id_temp
            );
        }
        bv_init_select_op_params();
        rc = sqlite3_exec(user_data_db_connection, sql, NULL, NULL, &errMessage);
        if(rc == SQLITE_OK) {
            count++;
            bv_free_byte_obj(temp_node_ptr->value_ptr);
            free(temp_node_ptr->value_ptr);
            free(temp_node_ptr);

            sprintf(temp, "-i %llu", byte_id);
            rc = bv_query_bytes(username, temp, 1, NULL, 1, &temp_node_ptr, &errMessage);
            if(rc == BV_OP_FAILED || temp_node_ptr == NULL) {
                *errMsg = errMessage;
                free(key_temp);
                free(value_temp);
                free(byte_id_temp);
                free(temp);
                free(sql);
                free(temp2);
                return BV_OP_FAILED;
            }
        }
    }

    bv_free_byte_obj(temp_node_ptr->value_ptr);
    free(temp_node_ptr->value_ptr);
    free(temp_node_ptr);

    free(key_temp);
    free(value_temp);
    free(byte_id_temp);
    free(temp);
    free(sql);
    free(temp2);
    bv_free_user_obj(&temp_user);

    if(count == data_count) {
        return BV_OP_SUCCESS;
    } else {
        //It is for partial insert or full failed.
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }
}

int bv_delete_bytedata(const char* username, size_t byte_id, const char* byte_data_key, char** errMsg) {
    sqlite3* user_data_db_connection;
    char* temp = malloc(300);
    char* temp2 = malloc(300);
    bv_user temp_user;
    char* key_temp = malloc(300);
    char* byte_id_temp = malloc(200);
    int rc;
    char* errMessage;
    char* sql = malloc(500);
    bv_node* temp_node_ptr = null;


    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        free(temp2);
        free(key_temp);
        free(byte_id_temp);
        free(sql);
        return BV_OP_FAILED;
    }

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        free(key_temp);
        free(byte_id_temp);
        free(sql);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(temp);
        free(temp2);
        free(key_temp);
        free(byte_id_temp);
        free(sql);
        return BV_OP_FAILED;
    }

    bv_encrypt_text(byte_data_key, temp_user.enc_key, temp);
    bv_escape_sqlite_value(key_temp, temp);
    sprintf(temp2, "%llu", byte_id);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(byte_id_temp, temp);

    bv_get_user_data_db_connection(&user_data_db_connection, username);
    sprintf(sql, "delete from %s where %s='%s' and %s='%s'",
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_BYTE_ID,
            byte_id_temp,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_KEY,
            key_temp
    );
    rc = sqlite3_exec(user_data_db_connection, sql, NULL, NULL, &errMessage);
    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        free(key_temp);
        free(byte_id_temp);
        free(sql);
        return BV_OP_FAILED;
    }

    bv_free_byte_obj(temp_node_ptr->value_ptr);
    free(temp_node_ptr->value_ptr);
    free(temp_node_ptr);

    free(temp);
    free(temp2);
    free(key_temp);
    free(byte_id_temp);
    free(sql);

    bv_free_user_obj(&temp_user);

    return BV_OP_SUCCESS;
}

/**
 * It just deletes single byte
 * And does not change the child byte's parent byte_id field.
 * So don't use it.
 * @param username
 * @param byte_id
 * @param errMsg
 * @return
 */
static int bv_delete_single_byte(const char* username, size_t byte_id, char** errMsg) {
    sqlite3* user_data_db_connection;
    char* temp = malloc(300);
    char* temp2 = malloc(300);
    bv_user temp_user;
    char* byte_id_temp = malloc(200);
    int rc;
    char* errMessage;
    char* sql = malloc(400);
    bv_node* temp_node_ptr = null;


    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        return BV_OP_FAILED;
    }

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        bv_free_user_obj(&temp_user);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        bv_free_user_obj(&temp_user);
        return BV_OP_FAILED;
    }

    sprintf(temp2, "%llu", byte_id);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(byte_id_temp, temp);

    bv_get_user_data_db_connection(&user_data_db_connection, username);
    sprintf(sql, "delete from %s where %s='%s'",
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_BYTE_ID,
            byte_id_temp
    );
    rc = sqlite3_exec(user_data_db_connection, sql, NULL, NULL, &errMessage);
    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        bv_free_user_obj(&temp_user);
        bv_free_byte_obj(temp_node_ptr->value_ptr);
        free(temp_node_ptr->value_ptr);
        free(temp_node_ptr);
        return BV_OP_FAILED;
    }

    sprintf(sql, "delete from %s where %s='%llu' ",
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID,
            byte_id
    );
    rc = sqlite3_exec(user_data_db_connection, sql, NULL, NULL, &errMessage);
    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        bv_free_user_obj(&temp_user);
        bv_free_byte_obj(temp_node_ptr->value_ptr);
        free(temp_node_ptr->value_ptr);
        free(temp_node_ptr);
        return BV_OP_FAILED;
    }

    free(temp);
    free(temp2);
    free(byte_id_temp);
    free(sql);
    bv_free_user_obj(&temp_user);
    bv_free_byte_obj(temp_node_ptr->value_ptr);
    free(temp_node_ptr->value_ptr);
    free(temp_node_ptr);

    return BV_OP_SUCCESS;
}

static int bv_delete_byte_with_recursive(const char *username, size_t byte_id, char **errMsg) {
    char* temp = malloc(200);
    bv_user temp_user;
    int rc;
    char* errMessage;
    bv_node* temp_node_ptr = null;
    size_t temp_id = 0;
    bv_node* temp_node2 = null;


    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        return BV_OP_FAILED;
    }

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(temp);
        bv_free_user_obj(&temp_user);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(temp);
        bv_free_user_obj(&temp_user);
        return BV_OP_FAILED;
    }

    free(temp);
    bv_free_user_obj(&temp_user);
    bv_free_byte_obj(temp_node_ptr->value_ptr);
    free(temp_node_ptr->value_ptr);
    free(temp_node_ptr);

    temp_node_ptr = null;
    rc = bv_get_direct_child_bytes(username, byte_id, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_SUCCESS && temp_node_ptr != null) {
        while(temp_node_ptr != null) {
            temp_id = ((bv_byte*)temp_node_ptr->value_ptr)->id;
            bv_free_byte_obj(temp_node_ptr->value_ptr);
            temp_node2 = temp_node_ptr;
            temp_node_ptr = temp_node_ptr->next;
            free(temp_node2->value_ptr);
            free(temp_node2);

            bv_delete_byte_with_recursive(username, temp_id, &errMessage);
        }
    }

    rc = bv_delete_single_byte(username, byte_id, &errMessage);
    if(rc != BV_OP_SUCCESS) {
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }

    return BV_OP_SUCCESS;
}


static int bv_delete_byte_without_recursive(const char* username, size_t byte_id, char** errMsg) {
    sqlite3* user_data_db_connection;
    char* temp = malloc(300);
    char* temp2 = malloc(300);
    bv_user temp_user;
    char* byte_id_temp = malloc(200);
    char* parent_byte_id_temp = malloc(200);
    int rc;
    char* errMessage;
    char* sql = malloc(400);
    bv_node* temp_node_ptr = null;


    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        free(parent_byte_id_temp);
        return BV_OP_FAILED;
    }

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        free(parent_byte_id_temp);
        bv_free_user_obj(&temp_user);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        free(parent_byte_id_temp);
        bv_free_user_obj(&temp_user);
        return BV_OP_FAILED;
    }

    sprintf(temp2, "%llu", byte_id);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(byte_id_temp, temp);

    sprintf(temp2, "%llu", ((bv_byte*)temp_node_ptr->value_ptr)->parent_byte_id);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(parent_byte_id_temp, temp);

    bv_get_user_data_db_connection(&user_data_db_connection, username);
    sprintf(sql, "update %s set %s='%s' where %s='%s'",
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_PARENT_BYTE_ID,
            parent_byte_id_temp,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_PARENT_BYTE_ID,
            byte_id_temp
    );
    rc = sqlite3_exec(user_data_db_connection, sql, NULL, NULL, &errMessage);
    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        free(parent_byte_id_temp);
        bv_free_user_obj(&temp_user);
        bv_free_byte_obj(temp_node_ptr->value_ptr);
        free(temp_node_ptr->value_ptr);
        free(temp_node_ptr);
        return BV_OP_FAILED;
    }

    rc = bv_delete_byte_with_recursive(username, byte_id, &errMessage);
    if(rc != BV_OP_SUCCESS) {
        *errMsg = errMessage;
        free(temp);
        free(temp2);
        free(byte_id_temp);
        free(sql);
        free(parent_byte_id_temp);
        bv_free_user_obj(&temp_user);
        bv_free_byte_obj(temp_node_ptr->value_ptr);
        free(temp_node_ptr->value_ptr);
        free(temp_node_ptr);
        return BV_OP_FAILED;
    }

    free(temp);
    free(temp2);
    free(byte_id_temp);
    free(sql);
    free(parent_byte_id_temp);
    bv_free_user_obj(&temp_user);
    bv_free_byte_obj(temp_node_ptr->value_ptr);
    free(temp_node_ptr->value_ptr);
    free(temp_node_ptr);

    return BV_OP_SUCCESS;
}

int bv_delete_user(const char* username, char** errMsg) {
    sqlite3* users_db_connection = NULL;
    char* errorMessage = NULL;
    char* sql = malloc(500);
    int rc;
    char* temp = malloc(110);
    char* username_temp = malloc(100);
    bv_user temp_user;

    rc = bv_get_user_info(username, &temp_user, &errorMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(username_temp);
        free(temp);
        free(sql);
        return BV_OP_FAILED;
    }

    bv_encrypt_text(username, BV_PRIMARY_KEY, temp);
    bv_escape_sqlite_value(username_temp, temp);

    bv_get_users_db_connection(&users_db_connection);
    sprintf(sql, "delete from %s where %s='%s' ",
            BV_USERS_DB_TABLE_NAME_USERS_INFO,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_USERNAME,
            username_temp
    );
    bv_init_select_op_params();
    rc = sqlite3_exec(users_db_connection, sql, NULL, NULL, &errorMessage);
    free(username_temp);
    free(temp);
    free(sql);
    bv_free_user_obj(&temp_user);
    if(rc != SQLITE_OK) {
        *errMsg = errorMessage;
        return BV_OP_FAILED;
    } else {
        return BV_OP_SUCCESS;
    }
}

int bv_delete_byte(const char* username, size_t* byte_ids, size_t byte_id_count, bv_bool recursive, void (*callback)(int code, size_t total_deleted, size_t total_bytes), int code, char** errMsg) {
    bv_user temp_user;
    int rc;
    char* errMessage = null;
    char* temp = malloc(200);
    size_t i = 0;
    bv_node* temp_node = null;
    size_t count_bytes_deleted = 0;

    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        return BV_OP_FAILED;
    }

    count_bytes_deleted = 0;
    for(i = 0; i<byte_id_count; i++) {
        sprintf(temp, "-i %llu", byte_ids[i]);
        rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node, &errMessage);
        if(rc != BV_OP_SUCCESS) {
            continue;
        }
        if(temp_node == null) {
            errMessage = "This byte does not exist";
            continue;
        }
        bv_free_byte_obj(temp_node->value_ptr);
        free(temp_node->value_ptr);
        free(temp_node);
        if(recursive) {
            rc = bv_delete_byte_with_recursive(username, byte_ids[i], &errMessage);
        } else {
            rc = bv_delete_byte_without_recursive(username, byte_ids[i], &errMessage);
        }
        if(rc == BV_OP_SUCCESS) {
            count_bytes_deleted++;
            if(callback != null) {
                callback(code, count_bytes_deleted, byte_id_count);
            }
        } else {
            continue;
        }
    }
    free(temp);

    bv_free_user_obj(&temp_user);

    if(count_bytes_deleted == byte_id_count) {
        return BV_OP_SUCCESS;
    } else {
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }
}

int bv_edit_byte_info(const char *username, size_t byte_id, const char *byte_info_column, const char *byte_info_column_value, char **errMsg) {
    sqlite3* user_data_db_connection;
    int rc;
    char* sql = NULL;
    char* errMessage;
    char* temp = null;
    bv_user temp_user;
    bv_node* temp_node_ptr = NULL;
    char* column_value_temp = NULL;

    if(!strcmpi(byte_info_column, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_CREATED) || !strcmpi(byte_info_column, BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID)) {
        *errMsg = "This field can't be edited";
        return BV_OP_FAILED;
    }

    temp = malloc(220);

    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        return BV_OP_FAILED;
    }

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(temp);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(temp);
        return BV_OP_FAILED;
    }

    sql = malloc(500);
    column_value_temp = malloc(300);
    bv_encrypt_text(byte_info_column_value, temp_user.enc_key, temp);
    bv_escape_sqlite_value(column_value_temp, temp);

    bv_get_user_data_db_connection(&user_data_db_connection, username);
    sprintf(sql, "update %s set %s='%s' where %s='%llu'",
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO,
            byte_info_column,
            column_value_temp,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID,
            byte_id
    );
    rc = sqlite3_exec(user_data_db_connection, sql, NULL, NULL, &errMessage);
    free(sql);
    free(temp);
    free(column_value_temp);
    bv_free_user_obj(&temp_user);
    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }

    return BV_OP_SUCCESS;
}

int bv_edit_user_info(const char* username, const char* user_info_column, const char* new_value, char** errMsg) {
    sqlite3* users_db_connection;
    int rc;
    char* sql = NULL;
    char* errMessage;
    char* temp = NULL;
    bv_user temp_user;
    char* column_temp = NULL;
    char* username_temp = NULL;

    if(!strcmpi(user_info_column, BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_USERNAME) || !strcmpi(user_info_column, BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_ENCRYPTION_KEY) || !strcmpi(user_info_column, BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_ID)) {
        *errMsg = "This field can't be edited";
        return BV_OP_FAILED;
    }

    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        return BV_OP_FAILED;
    }

    sql = malloc(500);
    temp = malloc(400);
    username_temp = malloc(100);
    column_temp = malloc(400);
    bv_encrypt_text(new_value, temp_user.enc_key, temp);
    bv_escape_sqlite_value(column_temp, temp);
    bv_encrypt_text(username, BV_PRIMARY_KEY, temp);
    bv_escape_sqlite_value(username_temp, temp);

    bv_get_users_db_connection(&users_db_connection);
    sprintf(sql, "update %s set %s='%s' where %s='%s' ",
            BV_USERS_DB_TABLE_NAME_USERS_INFO,
            user_info_column,
            column_temp,
            BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_USERNAME,
            username_temp
    );
    rc = sqlite3_exec(users_db_connection, sql, NULL, NULL, &errMessage);
    free(sql);
    free(temp);
    free(column_temp);
    free(username_temp);
    bv_free_user_obj(&temp_user);

    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }
    return BV_OP_SUCCESS;
}

static int bv_compare_function_ull(const void * a, const void * b)
{
    return (int)( *((size_t *)a) - *((size_t *)b) );
}

int bv_query_bytes(const char* username, const char* query_string, long long int limits, const char* order_by_clause, int is_byte_data_includes, bv_node** head_node, char** errMsg) {
    sqlite3* user_data_db_connection;
    bv_user temp_user;
    int rc;
    char* errMessage;
    char* where_cause = malloc(410);
    char* limits_clause = malloc(100);
    char* order_by_clause_ptr = malloc(100);
    char* temp = malloc(500);
    char* temp2 = null;
    char* sql = malloc(410);
    bv_select_op_code op_code = {2};
    bv_node* temp_node_ptr = NULL;
    bv_node* temp_node_ptr2 = NULL;
    bv_node* temp_node_ptr3 = NULL;
    bv_byte* byte_temp_ptr = NULL;
    size_t* ids_of_parent = null;
    size_t i = 0;
    size_t j = 0;
    size_t temp_byte_id = 0;
    bv_bool temp_bool = bv_false;

    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(where_cause);
        free(limits_clause);
        free(order_by_clause_ptr);
        free(temp);
        free(sql);
        return BV_OP_FAILED;
    }
    op_code.extra_data = (void*)temp_user.enc_key;

    query_string = (!query_string) ? ("all") : (query_string);
    if(!strcmpi(query_string, "all")) {
        strcpy(where_cause, " ");
    } else {
        if(query_string[0] == '-' && query_string[1] == 'i' && query_string[2] == ' ') {
            temp2 = malloc(strlen(query_string + 3) + 20);
            bv_trim(query_string + 3, temp2);
            bv_escape_sqlite_value(temp, temp2);
            sprintf(where_cause, " where %s='%s' ", BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID, temp);
            free(temp2);
            temp2 = null;
        } else if(query_string[0] == '-' && query_string[1] == 'n' && query_string[2] == ' ') {
            if(limits == 0) {
                *head_node = NULL;
                free(where_cause);
                free(limits_clause);
                free(order_by_clause_ptr);
                free(temp);
                free(sql);
                return BV_OP_SUCCESS;
            }
            rc = bv_query_bytes(username, "all", -1, order_by_clause, 0, &temp_node_ptr, &errMessage);
            if(rc == BV_OP_FAILED) {
                *errMsg = errMessage;
                free(where_cause);
                free(limits_clause);
                free(order_by_clause_ptr);
                free(temp);
                free(sql);
                return BV_OP_FAILED;
            }
            if(temp_node_ptr == NULL) {
                *head_node = NULL;
                free(where_cause);
                free(limits_clause);
                free(order_by_clause_ptr);
                free(temp);
                free(sql);
                return BV_OP_SUCCESS;
            }
            ids_of_parent = malloc(sizeof(size_t) * bv_ll_count(temp_node_ptr) );
            i = 0;
            temp2 = malloc(100);

            while( temp_node_ptr != NULL ) {
                byte_temp_ptr = temp_node_ptr->value_ptr;
                if( bv_search_query_in_str(query_string + 3, byte_temp_ptr->name) ) {
                    if(byte_temp_ptr->parent_byte_id == 0) {
                        temp_byte_id = 0;
                    } else {
                        rc = bv_get_grand_byte_id_of_byte(username, byte_temp_ptr->id, &temp_byte_id, &errMessage);
                        if(rc == BV_OP_FAILED) {
                            *errMsg = errMessage;
                            free(where_cause);
                            free(limits_clause);
                            free(order_by_clause_ptr);
                            free(temp);
                            free(sql);
                            return BV_OP_FAILED;
                        }
                    }
                    if(temp_byte_id == 0) {
                        temp_byte_id = byte_temp_ptr->id;
                    }
                    if(!bsearch(&temp_byte_id, ids_of_parent, i, sizeof(size_t), bv_compare_function_ull)) {
                        ids_of_parent[i] = temp_byte_id;
                        i++;
                        qsort(ids_of_parent, i, sizeof(size_t), bv_compare_function_ull);
                    }
                }
                bv_free_byte_obj(byte_temp_ptr);
                free(byte_temp_ptr);
                temp_node_ptr3 = temp_node_ptr;
                temp_node_ptr = temp_node_ptr->next;
                free(temp_node_ptr3);
            }
            if(i > 0) {
                temp_node_ptr = null;
                sprintf(temp2, "-i %llu", ids_of_parent[0]);
                rc = bv_query_bytes(username, temp2, 1, null, is_byte_data_includes, &temp_node_ptr, &errMessage);
                if(rc != BV_OP_SUCCESS || temp_node_ptr == null) {
                    *errMsg = "This byte id does not exist";
                    free(where_cause);
                    free(limits_clause);
                    free(order_by_clause_ptr);
                    free(temp);
                    free(sql);
                    return BV_OP_FAILED;
                }
                temp_node_ptr3 = temp_node_ptr;
                for(j = 1; j<i; j++) {
                    sprintf(temp2, "-i %llu", ids_of_parent[j]);
                    rc = bv_query_bytes(username, temp2, 1, null, is_byte_data_includes, &temp_node_ptr2, &errMessage);
                    if(rc != BV_OP_SUCCESS || temp_node_ptr2 == null) {
                        continue;
                    }
                    temp_node_ptr->next = temp_node_ptr2;
                    temp_node_ptr = temp_node_ptr2;
                }
                temp_node_ptr->next = NULL;
                *head_node = temp_node_ptr3;
            } else {
                *head_node = NULL;
            }

            free(where_cause);
            free(limits_clause);
            free(order_by_clause_ptr);
            free(sql);
            free(temp);
            free(ids_of_parent);
            free(temp2);

            bv_free_user_obj(&temp_user);

            return BV_OP_SUCCESS;
        } else {
            strcpy(where_cause, " ");
        }
    }

    if(limits < 0) {
        strcpy(limits_clause, " ");
    } else {
        sprintf(limits_clause, " limit %llu ", limits);
    }

    if(order_by_clause == NULL) {
        strcpy(order_by_clause_ptr, " ");
    } else {
        strcpy(order_by_clause_ptr, " ");
        strcat(order_by_clause_ptr, order_by_clause);
        strcat(order_by_clause_ptr, " ");
    }

    bv_get_user_data_db_connection(&user_data_db_connection, username);
    bv_init_select_op_params();
    sprintf(sql, "select * from %s %s %s %s", BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO, where_cause, order_by_clause_ptr, limits_clause);
    bv_init_select_op_params();//It must to call before any query operation.
    rc = sqlite3_exec(user_data_db_connection, sql, bv_select_data_callback, (void*)&op_code, &errMessage);
    free(where_cause);
    free(limits_clause);
    free(temp);
    free(order_by_clause_ptr);

    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        free(sql);
        return BV_OP_FAILED;
    }

    temp_node_ptr = bv_select_op_data_ptr;
    if(temp_node_ptr == NULL && bv_select_op_data_call_count == 0) {
        *head_node = NULL;
        free(sql);
        return BV_OP_SUCCESS;
    } else if(temp_node_ptr == NULL) {
        *errMsg = BV_ERROR_MSG_UNKNOWN_ERROR;
        free(sql);
        return BV_OP_FAILED;
    }
    *head_node = temp_node_ptr;
    if(!is_byte_data_includes) {
        free(sql);
        return BV_OP_SUCCESS;
    }
    op_code.op_code = 3;
    temp = malloc(400);
    temp2 = malloc(400);
    while (temp_node_ptr != NULL) {
        sprintf(temp, "%lld", ((bv_byte*)temp_node_ptr->value_ptr)->id);
        bv_encrypt_text(temp, op_code.extra_data, temp2);
        bv_escape_sqlite_value(temp, temp2);
        sprintf(sql, "select * from %s where %s='%s'", BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA, BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_BYTE_ID, temp);
        bv_init_select_op_params();//It must to call before any query operation.
        rc = sqlite3_exec(user_data_db_connection, sql, bv_select_data_callback, (void*)&op_code, &errMessage);
        if(rc != SQLITE_OK) {
            ((bv_byte*)temp_node_ptr->value_ptr)->data = NULL;
            ((bv_byte*)temp_node_ptr->value_ptr)->data_count = 0;
            temp_node_ptr = temp_node_ptr->next;
            continue;
        }
        if(bv_select_op_data_ptr == NULL) {
            ((bv_byte*)temp_node_ptr->value_ptr)->data = NULL;
            ((bv_byte*)temp_node_ptr->value_ptr)->data_count = 0;
        } else {
            ((bv_byte*)temp_node_ptr->value_ptr)->data = bv_select_op_data_ptr;
            ((bv_byte*)temp_node_ptr->value_ptr)->data_count = bv_select_op_data_call_count;
        }
        temp_node_ptr = temp_node_ptr->next;
    }
    free(sql);
    free(temp);
    free(temp2);
    bv_free_user_obj(&temp_user);
    return BV_OP_SUCCESS;
}

BV_DEPRECATED void bv_get_where_clause_from_str_for_query(const char* str, const char* column, const char* enc_key, char* dest) {
    char* temp_char_p = NULL;
    char* temp = malloc(300);
    char* temp2 = malloc(300);
    char* temp3 = malloc(strlen(str) + 10);

    strcpy(temp3, str);
    strcpy(dest, "");
    strcat(dest, " where ");
    enc_key = (enc_key == NULL) ? ("") : (enc_key);

    temp_char_p = strtok(temp3, " ");
    if(temp_char_p == NULL) { //When temp3 is empty or only space chars then strtok returns NULL.
        temp_char_p = "";
    }
    bv_encrypt_text(temp_char_p, enc_key, temp);
    bv_escape_sqlite_value_for_like_clause(temp2, temp);
    sprintf(temp, " %s like '%%%s%%' ", column, temp2);
    strcat(dest, temp);
    while ( (temp_char_p = strtok(NULL, " ")) != NULL ) {
        bv_encrypt_text(temp_char_p, enc_key, temp);
        bv_escape_sqlite_value_for_like_clause(temp2, temp);
        sprintf(temp, " or %s like '%%%s%%' ", column, temp2);
        strcat(dest, temp);
    }
    sprintf(temp, " escape '%s' ", BV_SQLITE_ESCAPE_CHAR);
    strcat(dest, temp);
    printf(" %s ", dest);
}

/**
 * This method does not check the key is exist or not
 * @param username
 * @param byte_id
 * @param key_current_name
 * @param key_new_name
 * @param errMsg
 * @return
 */
int bv_change_byte_data_key_name(const char* username, size_t byte_id, const char* key_current_name, const char* key_new_name, char** errMsg) {
    sqlite3* user_data_db_connection;
    int rc;
    char* sql = malloc(700);
    char* byte_id_temp = malloc(100);
    char* key_current_name_temp = malloc(300);
    char* key_new_name_temp = malloc(300);
    char* temp = malloc(300);
    char* temp2 = malloc(300);
    char* errMessage;
    bv_user temp_user;
    bv_node* temp_node_ptr = NULL;


    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(sql);
        free(byte_id_temp);
        free(key_current_name_temp);
        free(key_new_name_temp);
        free(temp);
        free(temp2);
        return BV_OP_FAILED;
    }

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(sql);
        free(byte_id_temp);
        free(key_current_name_temp);
        free(key_new_name_temp);
        free(temp);
        free(temp2);
        bv_free_user_obj(&temp_user);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(sql);
        free(byte_id_temp);
        free(key_current_name_temp);
        free(key_new_name_temp);
        free(temp);
        free(temp2);
        bv_free_user_obj(&temp_user);
        return BV_OP_FAILED;
    }

    sprintf(temp2, "%llu", byte_id);
    bv_encrypt_text(temp2, temp_user.enc_key, temp);
    bv_escape_sqlite_value(byte_id_temp, temp);
    bv_encrypt_text(key_current_name, temp_user.enc_key, temp);
    bv_escape_sqlite_value(key_current_name_temp, temp);
    bv_encrypt_text(key_new_name, temp_user.enc_key, temp);
    bv_escape_sqlite_value(key_new_name_temp, temp);

    bv_get_user_data_db_connection(&user_data_db_connection, username);
    sprintf(sql, "update %s set %s='%s' where %s='%s' and %s='%s' ",
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_KEY,
            key_new_name_temp,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_BYTE_ID,
            byte_id_temp,
            BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_KEY,
            key_current_name_temp
    );
    rc = sqlite3_exec(user_data_db_connection, sql, null, null, &errMessage);
    if(rc != SQLITE_OK) {
        *errMsg = errMessage;
        free(sql);
        free(byte_id_temp);
        free(key_current_name_temp);
        free(key_new_name_temp);
        free(temp);
        free(temp2);
        bv_free_user_obj(&temp_user);
        bv_free_byte_obj(temp_node_ptr->value_ptr);
        free(temp_node_ptr->value_ptr);
        free(temp_node_ptr);
        return BV_OP_FAILED;
    }


    free(sql);
    free(byte_id_temp);
    free(key_current_name_temp);
    free(key_new_name_temp);
    free(temp);
    free(temp2);
    bv_free_user_obj(&temp_user);
    bv_free_byte_obj(temp_node_ptr->value_ptr);
    free(temp_node_ptr->value_ptr);
    free(temp_node_ptr);
    return BV_OP_SUCCESS;
}

int bv_search_query_in_str(const char* query, const char* str) {
    char* temp_char_p = NULL;
    char* temp1 = NULL;
    char* temp2 = NULL;

    if(query[0] == '\0') {
        return 1;
    }

    temp1 = malloc(strlen(query) * 2);
    temp2 = malloc(strlen(str) + 10);
    bv_to_lower_str(query, temp1);
    bv_to_lower_str(str, temp2);

    temp_char_p = strtok(temp1, " ");
    if(temp_char_p == null) {
        return 1;
    }
    if( strstr(temp2, temp_char_p) ) {
        free(temp1);
        free(temp2);
        return 1;
    }
    while ( (temp_char_p = strtok(NULL, " ")) != NULL ) {
        if( strstr(temp2, temp_char_p) ) {
            free(temp1);
            free(temp2);
            return 1;
        }
    }
    free(temp1);
    free(temp2);
    return 0;
}

int bv_get_grand_byte_id_of_byte(const char* username, size_t byte_id, size_t* grand_byte_id_ptr, char** errMsg) {
    bv_user temp_user;
    int rc;
    char* errMessage = null;
    char* temp = malloc(300);
    bv_node* temp_node_ptr = null; //must
    bv_byte* temp_byte_ptr = null;
    unsigned long long temp_grand_byte_id = 0;
    unsigned long long temp_current_byte_id = 0;

    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        return BV_OP_FAILED;
    }

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = errMessage;
        free(temp);
        return BV_OP_FAILED;
    }
    if(temp_node_ptr == NULL) {
        *errMsg = "This byte does not exist";
        free(temp);
        return BV_OP_FAILED;
    }

    temp_byte_ptr = temp_node_ptr->value_ptr;
    if(temp_byte_ptr->parent_byte_id == 0) {
        *grand_byte_id_ptr = 0;
        free(temp);
        return  BV_OP_SUCCESS;
    } else {
        temp_current_byte_id = temp_byte_ptr->parent_byte_id;
    }

    free(temp_byte_ptr);
    free(temp_node_ptr);

    temp_node_ptr = null;
    while (1) {
        sprintf(temp, "-i %llu", temp_current_byte_id);
        rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node_ptr, &errMessage);
        if(rc == BV_OP_FAILED) {
            *errMsg = errMessage;
            free(temp);
            return BV_OP_FAILED;
        }
        if(temp_node_ptr == null) {
            temp_grand_byte_id = 0;
        } else {
            temp_byte_ptr = temp_node_ptr->value_ptr;
            temp_grand_byte_id = temp_byte_ptr->parent_byte_id;
        }
        if(temp_grand_byte_id == 0) {
            break;
        } else {
            temp_current_byte_id = temp_grand_byte_id;
        }

        if(temp_node_ptr != null) {
            free(temp_node_ptr->value_ptr);
            free(temp_node_ptr);
            temp_node_ptr = null;
        }
    }

    bv_free_user_obj(&temp_user);

    *grand_byte_id_ptr = temp_current_byte_id;
    free(temp);
    if(temp_node_ptr != null) {
        free(temp_node_ptr->value_ptr);
        free(temp_node_ptr);
    }
    return  BV_OP_SUCCESS;
}


void bv_escape_xml_value(const char* value, char* dest) {
    strcpy(dest, value);
    bv_replace_char_in_str(dest, '"', '\'');
}

static int bv_backup_single_byte(FILE* output_stream, const char* username, size_t byte_id, bv_bool recursive, char** errMsg) {
    int rc;
    char* errMessage = null;
    char* temp = malloc(BV_BYTE_DATA_VALUE_LENGTH + 200);
    char* temp2 = malloc(200);
    bv_node* temp_node = null;
    bv_node* temp_node2 = null;
    bv_byte* temp_byte = null;
    size_t i = 0;
    size_t temp_id = 0;

    sprintf(temp, "-i %llu", byte_id);
    rc = bv_query_bytes(username, temp, 1, null, 1, &temp_node, &errMessage);
    if(rc != BV_OP_SUCCESS) {
        free(temp);
        free(temp2);
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }
    if(temp_node == null) {
        free(temp);
        free(temp2);
        *errMsg = "This byte does not exist";
        return BV_OP_FAILED;
    }

    temp_byte = temp_node->value_ptr;

    fprintf(output_stream, "<byte ");
    sprintf(temp2, "%llu", temp_byte->id);
    bv_escape_xml_value(temp2, temp);
    fprintf(output_stream, "id=\"%s\" ", temp);
    bv_escape_xml_value(temp_byte->name, temp);
    fprintf(output_stream, "name=\"%s\" ", temp);
    sprintf(temp2, "%llu", temp_byte->parent_byte_id);
    bv_escape_xml_value(temp2, temp);
    fprintf(output_stream, "parent-byte-id=\"%s\" ", temp);
    bv_format_date(temp_byte->date_created, temp2);
    bv_escape_xml_value(temp2, temp);
    fprintf(output_stream, "date-created=\"%s\" ", temp);
    bv_format_date(temp_byte->date_last_modified, temp2);
    bv_escape_xml_value(temp2, temp);
    fprintf(output_stream, "date-last-modified=\"%s\" ", temp);
    bv_escape_xml_value(temp_byte->last_modifying_text, temp);
    fprintf(output_stream, "last-modifying-text=\"%s\" ", temp);
    fprintf(output_stream, ">\n");

    for(i = 0; i<temp_byte->data_count; i++) {
        fprintf(output_stream, "<bytedata ");
        bv_escape_xml_value(temp_byte->data[i].key, temp);
        fprintf(output_stream, "key=\"%s\" ", temp);
        bv_escape_xml_value(temp_byte->data[i].value, temp);
        fprintf(output_stream, "value=\"%s\" ", temp);
        fprintf(output_stream, "/>\n");
    }

    bv_free_byte_obj(temp_node->value_ptr);
    free(temp_node->value_ptr);
    free(temp_node);
    temp_node = null;
    free(temp); //it is important before recursive
    free(temp2); //it is important before recursive

    if(recursive) {
        rc = bv_get_direct_child_bytes(username, byte_id, 0, &temp_node, &errMessage);
        if(rc == BV_OP_SUCCESS && temp_node != null) {
            while(temp_node != null) {
                temp_id = ((bv_byte*)temp_node->value_ptr)->id;
                bv_free_byte_obj(temp_node->value_ptr);
                temp_node2 = temp_node;
                temp_node = temp_node->next;
                free(temp_node2->value_ptr);
                free(temp_node2);

                bv_backup_single_byte(output_stream, username, temp_id, recursive, &errMessage);
            }
        }
    }

    fprintf(output_stream, "</byte>\n");

    return BV_OP_SUCCESS;
}


int bv_backup_byte(FILE* output_stream, const char* username, size_t* byte_ids, size_t byte_id_count, bv_bool recursive, void (*callback)(int code, size_t total_backed_up, size_t total_bytes), int code, char** errMsg) {
    bv_user temp_user;
    int rc;
    char* errMessage = null;
    char* temp = malloc(300);
    char temp2[100];
    size_t i = 0;
    bv_node* temp_node = null;
    size_t count_bytes_backed_up = 0;

    rc = bv_get_user_info(username, &temp_user, &errMessage);
    if(rc == BV_OP_FAILED) {
        *errMsg = "User does not exist";
        free(temp);
        return BV_OP_FAILED;
    }

    output_stream = (output_stream) ? (output_stream) : (stdout);

    fprintf(output_stream, "%s\n\n", "<?xml version=\"1.0\" encoding=\"utf-8\" ?>");
    fprintf(output_stream, "<backup ");
    bv_escape_xml_value(BV_APP_NAME, temp);
    fprintf(output_stream, "generated-by=\"%s\" ", temp);
    bv_escape_xml_value(BV_APP_VERSION_NAME, temp);
    fprintf(output_stream, "app-version=\"%s\" ", temp);
    bv_format_date(time(NULL), temp2);
    bv_escape_xml_value(temp2, temp);
    fprintf(output_stream, "date=\"%s\" ", temp);
    bv_escape_xml_value(username, temp);
    fprintf(output_stream, "username=\"%s\" ", temp);
    fprintf(output_stream, ">\n");

    count_bytes_backed_up = 0;
    for(i = 0; i<byte_id_count; i++) {
        sprintf(temp, "-i %llu", byte_ids[i]);
        rc = bv_query_bytes(username, temp, 1, NULL, 0, &temp_node, &errMessage);
        if(rc != BV_OP_SUCCESS) {
            continue;
        }
        if(temp_node == null) {
            errMessage = "This byte does not exist";
            continue;
        }
        bv_free_byte_obj(temp_node->value_ptr);
        free(temp_node->value_ptr);
        free(temp_node);
        rc = bv_backup_single_byte(output_stream, username, byte_ids[i], recursive, &errMessage);
        if(rc == BV_OP_SUCCESS) {
            count_bytes_backed_up++;
            if(callback != null) {
                callback(code, count_bytes_backed_up, byte_id_count);
            }
        }
    }
    fprintf(output_stream, "</backup>\n");
    free(temp);

    bv_free_user_obj(&temp_user);

    if(count_bytes_backed_up == byte_id_count) {
        return BV_OP_SUCCESS;
    } else {
        *errMsg = errMessage;
        return BV_OP_FAILED;
    }
}




















