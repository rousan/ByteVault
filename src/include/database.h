/**
 * Here the size limits are :
 *  1. USERNAME -> 1<=size<= 20 CHARS
 *  2. PASSWORD -> 5<=size<=20
 *  3. ENC_KEY -> size=33 CHARS
 *  4. BYTE_NAME, KEY, LAST_MODIFYING_TEXT -> 1<=size<= 100
 *  5. VALUE -> 1<=size<=BV_BYTE_DATA_VALUE_LENGTH
 */

#ifndef BYTEVAULT_DATABASE_H
#define BYTEVAULT_DATABASE_H

#include "common.h"
#include "sqlite/sqlite3.h"
#include "linked_list.h"
#include "user.h"

#define BV_FILE_USERS_DB "users.db"
#define BV_APPEND_STRING_USERS_DATA_DB "_data.db"
#define BV_BYTE_DATA_VALUE_LENGTH (1024 * 1024 * 5)

/**
 * It is used to lock the db files.
 */
#define BV_DB_PASSWORD "8593cff5220f4c77a29b883a2756c00es"//33 bytes

#define BV_USERS_DB_TABLE_NAME_USERS_INFO "users_info"
#define BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_USERNAME "username"
#define BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_PASSWORD "password"
#define BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_ENCRYPTION_KEY "encryption_key"
#define BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_HINTS "hints"
#define BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES "security_question"
#define BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_SEC_QUES_ANS "security_question_answer"
#define BV_USERS_DB_TABLE_NAME_USERS_INFO_COLUMN_NAME_ID "id"

#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO "bytes_info"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_NAME "name"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_PARENT_BYTE_ID "parent_byte_id"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_CREATED "date_created"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_DATE_LAST_MODIFIED "date_last_modified"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_LAST_MODIFYING_TEXT "last_modifying_text"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_INFO_COLUMN_NAME_ID "id"

#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA "bytes_data"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_KEY "key"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_VALUE "value"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_BYTE_ID "byte_id"
#define BV_USERS_DATA_DB_TABLE_NAME_BYTES_DATA_COLUMN_NAME_ID "id"

#define BV_OP_SUCCESS 0xf
#define BV_OP_FAILED 0xe

#define BV_SQLITE_ESCAPE_CHAR "o"

extern bv_user* bv_logged_in_user;

typedef struct {
    int op_code;
    void* extra_data;
} bv_select_op_code;

typedef struct {
    const char* username;
    sqlite3* db_conn;
} bv_user_data_db_conn_struct;

void bv_escape_xml_value(const char* value, char* dest);
void bv_get_users_db_connection(sqlite3** connection);
void bv_get_user_data_db_connection(sqlite3 **connection, const char *username);
void bv_close_db_connections();
void bv_escape_sqlite_value(char* dest, const char* value);
BV_DEPRECATED void bv_escape_sqlite_value_for_like_clause(char* dest, const char* value);
int bv_insert_user(bv_user* user, char** errorMsg);
int bv_get_user_info(const char* username, bv_user* user, char** errMsg);
void bv_open_db_connections();
int bv_insert_byte(const char* username, bv_byte* byte, char** errMsg);
int bv_insert_byte_data(const char* username, size_t byte_id, bv_data* data, size_t data_count, char** errMsg);
int bv_query_bytes(const char* username, const char* query_string, long long int limits, const char* order_by_clause, int is_byte_data_includes, bv_node** head_node, char** errMsg);
BV_DEPRECATED void bv_get_where_clause_from_str_for_query(const char* str, const char* column, const char* enc_key, char* dest);
int bv_search_query_in_str(const char* query, const char* str);
int bv_edit_byte_info(const char *username, size_t byte_id, const char *byte_info_column, const char *byte_info_column_value, char **errMsg);
int bv_edit_user_info(const char* username, const char* user_info_column, const char* new_value, char** errMsg);
int bv_get_grand_byte_id_of_byte(const char* username, size_t byte_id, size_t* grand_byte_id_ptr, char** errMsg);
int bv_get_direct_child_bytes(const char* username, size_t byte_id, int is_byte_data_includes, bv_node** head_node, char** errMsg);
int bv_backup_byte(FILE* output_stream, const char* username, size_t* byte_ids, size_t byte_id_count, bv_bool recursive, void (*callback)(int code, size_t total_backed_up, size_t total_bytes), int code, char** errMsg);
int bv_delete_bytedata(const char* username, size_t byte_id, const char* byte_data_key, char** errMsg);
int bv_delete_byte(const char* username, size_t* byte_ids, size_t byte_id_count, bv_bool recursive, void (*callback)(int code, size_t total_deleted, size_t total_bytes), int code, char** errMsg);
int bv_delete_user(const char* username, char** errMsg);
int bv_change_byte_data_key_name(const char* username, size_t byte_id, const char* key_current_name, const char* key_new_name, char** errMsg);


#endif //BYTEVAULT_DATABASE_H
















