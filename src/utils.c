//
// Created by Ariyan Khan on 05-10-2016.
//

#include <windows.h>
#include <ctype.h>
#include <conio.h>
#include <signal.h>
#include <stdarg.h>

#include "include/utils.h"
#include "include/logger.h"
#include "include/constants.h"
#include "include/database.h"
#include "include/sqlite/sqlite3.h"
#include "include/viewer.h"
#include "include/command_actions.h"



void bv_resolve_str(const char** ptr) {
    *ptr = (*ptr == NULL) ? ("null") : (*ptr);
}

void bv_exit(int code) {
    exit(code);
}


void bv_free(void* mem_block) {
    if(mem_block != null) {
        free(mem_block);
    }
}

void bv_clear_screen() {
    system("cls");
}

void bv_set_app_window_title() {
    char* ptr = malloc(strlen(BV_APP_NAME) + 20);
    strcpy(ptr, "title ");
    strcat(ptr, BV_APP_NAME);
    system(ptr);
    free(ptr);
}

void bv_get_unique_backup_folder_name(char* dest) {
    struct tm* today = null;
    time_t time_in_sec = 0;

    time_in_sec = time(NULL);
    today = localtime(&time_in_sec);
    sprintf(dest, "time(%d-%d-%d-%ld) date(%d-%d-%d)", today->tm_hour, today->tm_min, today->tm_sec, clock(), today->tm_mday, today->tm_mon, today->tm_year);
}

void bv_open_path_in_explorer(const char* path) {
    char* temp = null;

    temp = malloc(strlen(path) + 100);
    sprintf(temp, "explorer.exe \"%s\"", path);
    system(temp);
}


void bv_get_unique_temp_file_path(char *dest, size_t file_name_length) {
    char* temp = malloc(file_name_length + 20);

    do {
        bv_generate_random_key(temp, file_name_length);
        GetTempPath(MAX_PATH, dest);
        if(dest[strlen(dest) - 1] == '\\' || dest[strlen(dest) - 1] == '/') {
            dest[strlen(dest) - 1] = '\0';
        }
        strcat(dest, "\\");
        strcat(dest, temp);
    } while (bv_is_existing_file(dest));

    free(temp);
}

void bv_get_absolute_path(const char* path, char* dest) {
    bv_bool is_abs = bv_false;
    char* current_dir = null;
    char* temp = null;
    bv_bool temp_bool = bv_false;

    if(strchr(path, ':')) {
        is_abs = bv_true;
    } else {
        is_abs = bv_false;
    }

    if(is_abs) {
        strcpy(dest, path);
        if(dest[strlen(dest) - 1] == '\\' || dest[strlen(dest) - 1] == '/') {
            dest[strlen(dest) - 1] = '\0';
        }
    } else {
        temp = malloc(strlen(path) + 10);
        strcpy(temp, path);
        if(temp[0] == '\\' || temp[0] == '/') {
            temp[0] = '\0';
            temp++;
            temp_bool = bv_true;
        } else {
            temp_bool = bv_false;
        }
        if(temp[strlen(temp) - 1] == '\\' || temp[strlen(temp) - 1] == '/') {
            temp[strlen(temp) - 1] = '\0';
        }
        current_dir = malloc(MAX_PATH + 20);
        GetCurrentDirectory(MAX_PATH, current_dir);
        if(current_dir[strlen(current_dir) - 1] == '\\' || current_dir[strlen(current_dir) - 1] == '/') {
            current_dir[strlen(current_dir) - 1] = '\0';
        }
        strcpy(dest, current_dir);
        strcat(dest, "\\");
        strcat(dest, temp);
        free(current_dir);
        if(temp_bool)
            temp--;
        free(temp);

    }
}


void bv_get_path_parts(const char* path, char* folder_dest, char* filename_dest) {
    char* temp = malloc(MAX_PATH + 200);
    char* last_slash_pos = null;
    char* last_backslash_pos = null;
    char* last_forwardslash_pos = null;

    bv_get_absolute_path(path, temp);
    last_backslash_pos = strrchr(temp, '\\');
    last_forwardslash_pos = strrchr(temp, '/');
    if(last_backslash_pos > last_forwardslash_pos) {
        last_slash_pos = last_backslash_pos;
    } else {
        last_slash_pos = last_forwardslash_pos;
    }

    if(last_slash_pos == null) {
        strcpy(filename_dest, temp);
        strcpy(folder_dest, "");
    } else {
        strcpy(filename_dest, last_slash_pos + 1);
        while (last_slash_pos >= temp) {
            if(last_slash_pos[0] == '\\' || last_slash_pos[0] == '/') {
                last_slash_pos[0] = '\0';
                last_slash_pos--;
            } else {
                break;
            }
        }
        strcpy(folder_dest, temp);
    }
    free(temp);
}


int bv_copy_file(const char* source_file, const char* dest_file, bv_bool over_written, char** errMsg) {
    FILE* s_file = null;
    FILE* d_file = null;
    unsigned char* buff = null;
    fpos_t temp_fpos = 0;
    size_t s_file_size = 0;
    const size_t max_buff_size = 1024 * 1024 * 3;
    size_t buff_size = 0;
    size_t read_bytes = 0;
    size_t written_bytes = 0;

    /*
     * It is important to use 'rb' and 'wb' instead of 'r' or 'w' when using fread and fwrite
     * because in 'w' mode all 0xA('\n') byte will be written as 0xD('\r') character in windows and
     * any others extra data will be written if the source byte is greater than 127.
     * And in 'r' mode all byte will parsed as text, Even if we use fread or fwrite function.
     * SO USE 'RB' AND 'RW' IF YOU WANT TO USE FWRITE OR FREAD FUNCTION.
     * So:
     *  fwrite, fread: 'rb', 'rw', 'rb+', 'rw+' etc
     *  gets, scanf etc: 'r', 'b', 'r+', 'w+' etc;
     */

    if(!over_written && bv_is_existing_file(dest_file)) {
        *errMsg = "Destination file is already exist";
        return BV_OP_FAILED;
    }

    s_file = fopen(source_file, "rb");
    d_file = fopen(dest_file, "wb");

    if(s_file == null || d_file == null) {
        *errMsg = "Can't open file";
        return BV_OP_FAILED;
    }

    fseek(s_file, 0, SEEK_END);
    fgetpos(s_file, &temp_fpos);
    fseek(s_file, 0, SEEK_SET);
    s_file_size = (size_t)temp_fpos;
    if(s_file_size < max_buff_size) {
        buff_size = s_file_size;
    } else {
        buff_size = max_buff_size;
    }
    if(buff_size == 0) {
        buff_size = 10;
    }
    buff = malloc(buff_size);
    while (!feof(s_file)) {
        read_bytes = fread(buff, 1, buff_size, s_file);
        written_bytes = 0;
        while(written_bytes < read_bytes) {
            written_bytes += fwrite(buff + written_bytes, 1, read_bytes - written_bytes, d_file);
        }
    }
    fclose(s_file);
    fflush(d_file);
    fclose(d_file);

    free(buff);
    return BV_OP_SUCCESS;
}

char* bv_replace_char_in_str(char* str, char char_to, char char_with) {
    char* found_ptr = null;
    char* temp = null;

    temp = str;
    while ( temp[0] != '\0' && (found_ptr = strchr(temp, char_to)) != NULL) {
        found_ptr[0] = char_with;
        temp = found_ptr + 1;
    }
    return str;
}

void bv_callback_on_app_start() {
    bv_console_text_color(BV_NORMAL_COLOR);
    atexit(bv_callback_on_app_exit);
    char temp[100];
    bv_get_root_folder_path(temp, sizeof(temp)/ sizeof(char));
    strcat(temp, "\\");
    strcat(temp, (BV_FOLDER_DATA));
    if(!bv_is_existing_folder(temp)) {
        CreateDirectory(temp, NULL);
    }
    bv_open_db_connections();
    bv_set_app_window_title();
}

void bv_get_app_cache_folder_path(char* dest, size_t size) {
    strncpy(dest, "C:\\ByteVault\\ByteVault-1.0.0\\bin\\cache", size - 1); //create cache folder if not exists
}

bv_bool bv_is_existing_file(const char* filePath) {
    DWORD dword_temp;
    dword_temp = GetFileAttributes(filePath);
    if((dword_temp != INVALID_FILE_ATTRIBUTES) && !(dword_temp & FILE_ATTRIBUTE_DIRECTORY)) {
        return bv_true;
    } else {
        return bv_false;
    }
}

bv_bool bv_is_existing_folder(const char* filePath) {
    DWORD dword_temp;
    dword_temp = GetFileAttributes(filePath);
    if((dword_temp != INVALID_FILE_ATTRIBUTES) && (dword_temp & FILE_ATTRIBUTE_DIRECTORY)) {
        return bv_true;
    } else {
        return bv_false;
    }
}

void bv_get_root_folder_path(char* dest, size_t size) {
    strncpy(dest, "C:\\ByteVault\\ByteVault-1.0.0", size - 1);  //Here path will be the actual path of exe and the ../ path
}


void bv_get_backups_folder_path(char* dest, size_t size) {
    strncpy(dest, "C:\\ByteVault\\ByteVault-1.0.0\\backups", size - 1);  //create backups folder if not exists
}

void bv_callback_on_app_exit(void) {
    bv_close_db_connections();
}

/**
 * This routine reads password with masking technique from console.
 * @param dest
 * @param size
 */
void bv_read_password_from_console(char* dest, size_t size, int flag) {
    size_t i;
    int current_char;
    char temp[5];
    int first_time_action;

    if(size == 0) {
        return;
    } else if(size == 1) {
        dest[0] = '\0';
        return;
    }
    fseek(stdin, 0, SEEK_END);
    strcpy(dest, "");
    i = 0;
    first_time_action = 0; //first time its value is 0 must.
    while(1) {
        current_char = getch();
        if(bv_is_valid_password_char(current_char)) {
            sprintf(temp, "%c", current_char);
            strcat(dest, temp);
            if(flag == 1) {
                if(dest[0] != '\0' && bv_starts_with_str(BV_SPECIAL_COMMAND_ACTION_EXIT_INPUT_STRING, dest) || bv_starts_with_str(BV_SPECIAL_COMMAND_ACTION_PREV_INPUT_STRING, dest)) {
                    printf("%c", current_char);
                    first_time_action = 1;
                } else {
                    if(first_time_action == 1) {
                        bv_print_char('\b', (int)(strlen(dest) - 1));
                        bv_print_char('*', (int)strlen(dest));
                    } else {
                        printf("*");
                    }
                    first_time_action = 0;
                }

            } else {
                printf("*");
            }
            i++;
            if(i >= size - 1) {
                break;
            }
        } else if(current_char == VK_BACK) {
            if(strlen(dest) >= 1) {
                dest[strlen(dest) - 1] = '\0';
                i--;
                printf("\b \b");
                if(flag == 1) {
                    if(dest[0] != '\0' && bv_starts_with_str(BV_SPECIAL_COMMAND_ACTION_EXIT_INPUT_STRING, dest) || bv_starts_with_str(BV_SPECIAL_COMMAND_ACTION_PREV_INPUT_STRING, dest)) {
                        bv_print_char('\b', (int)strlen(dest));
                        printf("%s", dest);
                        first_time_action = 1;
                    } else {
                        first_time_action = 0;
                    }
                }
            }
        } else if(current_char == '\n' || current_char == '\r') {
            printf("\n");
            break;
        }
    }
}

unsigned long long bv_max_ull(size_t ln, ...) {
    va_list list;
    unsigned long long max = 0;
    size_t i = 0;
    unsigned long long curr;

    if(ln == 0) {
        return 0;
    }
    va_start(list, ln);
    max = va_arg(list, unsigned long long);
    for(i = 0; i<ln - 1; i++) {
        curr = va_arg(list, unsigned long long);
        if(curr > max) {
            max = curr;
        }
    }
    return max;
}

unsigned long long bv_min_ull(size_t ln, ...) {
    va_list list;
    unsigned long long min = 0;
    size_t i = 0;
    unsigned long long curr;

    if(ln == 0) {
        return 0;
    }
    va_start(list, ln);
    min = va_arg(list, unsigned long long);
    for(i = 0; i<ln - 1; i++) {
        curr = va_arg(list, unsigned long long);
        if(curr < min) {
            min = curr;
        }
    }
    return min;
}

bv_bool bv_is_valid_password_char(int chr) {
    if(isalnum(chr) || ispunct(chr) || chr == ' ') {
        return bv_true;
    } else {
        return bv_false;
    }
}

void bv_to_lower_str(const char* str, char* dest) {
    size_t i = 0;
    size_t ln = strlen(str);
    char temp[5];

    strcpy(dest, "");
    for (i=0; i<ln; i++) {
        sprintf(temp, "%c", tolower(str[i]));
        strcat(dest, temp);
    }
}

void bv_to_upper_str(const char* str, char* dest) {
    size_t i = 0;
    size_t ln = strlen(str);
    char temp[5];

    strcpy(dest, "");
    for (i=0; i<ln; i++) {
        sprintf(temp, "%c", toupper(str[i]));
        strcat(dest, temp);
    }
}

int bv_get_random_number(int min, int max, unsigned int seed) {
    if(seed > 0) {
        srand((unsigned int)clock() + (unsigned int)time(null));
    }
    return (min + rand() % (max - min + 1));
}

/**
 *
 * @param dest length assumes (uuid_size) + 1 size
 * @param uuid_size It is the byte of size of UUID
 */
void bv_generate_random_key(char *dest, size_t key_size) {
    size_t i = 0;
    char temp[5];
    char u_alpha_chr;
    char l_alpha_chr;
    char num_chr;
    int rc;

    strcpy(dest, "");
    srand((unsigned int)clock() + (unsigned int)time(null));
    for (i = 0; i < key_size; i++) {
        u_alpha_chr = (char)bv_get_random_number('A', 'Z', 0);
        l_alpha_chr = (char)bv_get_random_number('a', 'z', 0);
        num_chr = (char)bv_get_random_number('0', '9', 0);
        rc = bv_get_random_number(0, 2, 0);
        if(rc == bv_get_random_number(0, 2, 0)) {
            sprintf(temp, "%c", u_alpha_chr);
        } else if(rc == bv_get_random_number(0, 2, 0)) {
            sprintf(temp, "%c", l_alpha_chr);
        } else if(rc == bv_get_random_number(0, 2, 0)) {
            sprintf(temp, "%c", num_chr);
        } else {
            sprintf(temp, "%c", num_chr);
        }
        strcat(dest, temp);
    }
}

void bv_trim_start(const char* str, char* dest) {
    while(str[0] != '\0' && isspace(str[0])) {
        str++;
    }
    strcpy(dest, str);
}

void bv_trim_end(const char* str, char* dest) {
    char* curr_char_ptr = null;
    char* temp = null;
    if(str[0] == '\0') {
        strcpy(dest, "");
        return;
    }

    temp = malloc(strlen(str) + 10);
    strcpy(temp, str);
    curr_char_ptr = temp + (strlen(temp) - 1);
    while(curr_char_ptr >= temp && isspace(curr_char_ptr[0])) {
        curr_char_ptr--;
    }
    curr_char_ptr[1] = '\0';
    strcpy(dest, temp);
    free(temp);
}

void bv_trim(const char* str, char* dest) {
    char* temp = null;
    temp = malloc(strlen(str) + 10);
    bv_trim_start(str, temp);
    bv_trim_end(temp, dest);
    free(temp);
}


bv_bool bv_starts_with_str(const char* str, const char* sub_str) {
    if(sub_str[0] == '\0') {
        return bv_true;
    }
    if(str[0] == '\0') {
        return bv_false;
    }
    if(strstr(str, sub_str) == str) {
        return bv_true;
    } else {
        return bv_false;
    }
}

bv_bool bv_ends_with_str(const char* str, const char* sub_str) {
    const char* temp_str = null;
    const char* temp_sub_str = null;

    if(strlen(str) < strlen(sub_str)) {
        return bv_false;
    }
    temp_str = str + (strlen(str) - 1);
    temp_sub_str = sub_str + (strlen(sub_str) - 1);
    while (temp_sub_str >= sub_str) {
        if(temp_str[0] != temp_sub_str[0]) {
            return bv_false;
        }
        temp_str--;
        temp_sub_str--;
    }
    return bv_true;
}

char* bv_split_str(const char* str, const char* delim, char*** dest_ptr, size_t* count) {
    char* temp = malloc(strlen(str) + 10);
    char* char_ptr = null;
    char** dest = null;

    *count = 0;
    strcpy(temp, str);
    char_ptr = strtok(temp, delim);
    if(char_ptr == null) {
        *dest_ptr = null;
        *count = 0;
        free(temp);
        return null;
    } else {
        dest = malloc(sizeof(char*) * 1);
        dest[*count] = char_ptr;
        (*count)++;
    }
    while( (char_ptr = strtok(NULL, delim)) != NULL ) {
        dest = realloc(dest, sizeof(char*) * (*count + 1));
        dest[*count] = char_ptr;
        (*count)++;
    }
    *dest_ptr = dest;
    return temp; //returns the address to free later
    //Dont free temp, it is the output
}



























