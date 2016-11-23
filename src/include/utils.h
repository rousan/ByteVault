//
// Created by Ariyan Khan on 05-10-2016.
//

#ifndef BYTEVAULT_UTILS_H
#define BYTEVAULT_UTILS_H

#include "common.h"
#include "viewer.h"

void bv_resolve_str(const char** ptr);
void bv_callback_on_app_start(void);
void bv_callback_on_app_exit(void);
void bv_get_root_folder_path(char* dest, size_t size);
void bv_exit(int code);
void bv_read_password_from_console(char* dest, size_t size, int flag);
int bv_get_random_number(int min, int max, unsigned int seed);
void bv_to_lower_str(const char* str, char* dest);
void bv_to_upper_str(const char* str, char* dest);
void bv_trim(const char* str, char* dest);
void bv_trim_end(const char* str, char* dest);
void bv_trim_start(const char* str, char* dest);
bv_bool bv_starts_with_str(const char* str, const char* sub_str);
char* bv_split_str(const char* str, const char* delim, char*** dest_ptr, size_t* count);
bv_bool bv_is_valid_password_char(int chr);
unsigned long long bv_max_ull(size_t ln, ...);
unsigned long long bv_min_ull(size_t ln, ...);
void bv_generate_random_key(char *dest, size_t key_size);
void bv_get_app_cache_folder_path(char* dest, size_t size);
bv_bool bv_is_existing_file(const char* filePath);
bv_bool bv_is_existing_folder(const char* filePath);
void bv_free(void* mem_block);
void bv_get_backups_folder_path(char* dest, size_t size);
void bv_get_unique_backup_folder_name(char* dest);
void bv_open_path_in_explorer(const char* path);
char* bv_replace_char_in_str(char* str, char char_to, char char_with);
void bv_get_unique_temp_file_path(char *dest, size_t file_name_length);
int bv_copy_file(const char* source_file, const char* dest_file, bv_bool over_written, char** errMsg);
void bv_get_absolute_path(const char* path, char* dest);
void bv_get_path_parts(const char* path, char* folder_dest, char* filename_dest);
bv_bool bv_ends_with_str(const char* str, const char* sub_str);
void bv_clear_screen();
void bv_set_app_window_title();

#define BV_STDIN_STRING_MAX_SIZE 500

#endif //BYTEVAULT_UTILS_H
