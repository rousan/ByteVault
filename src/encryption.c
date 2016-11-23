//
// Created by Ariyan Khan on 05-10-2016.
//

#include "include/encryption.h"
#include "include/database.h"

const char* const BV_PRIMARY_KEY = "3fb6547c6ded423594d8dfeeb30b6a341"; //33 bytes, Don't change this.

void bv_encrypt_byte_block(const void* target, size_t target_size, const void* key, size_t key_size, void* dest) {
    if (key_size == 0) {
        memcpy(dest, target, target_size);
        return;
    }
    size_t i = 0;
    size_t j = 0;
    for(i = 0, j = 0; i<target_size; i++, j++) {
        if (j == key_size)
            j = 0;
        ((unsigned char*)dest)[i] = (((const unsigned char*)target)[i]) ^ (((const unsigned char*)key)[j]);
    }
}

void bv_decrypt_byte_block(const void* target, size_t target_size, const void* key, size_t key_size, void* dest) {
    bv_encrypt_byte_block(target, target_size, key, key_size, dest);
}

int bv_encrypt_file(const char* source_file, const char* dest_file, const char* enc_key, int code, void (*callback)(int code, size_t bytes_encrypted, size_t total_bytes), char** errMsg) {
    FILE* s_file = null;
    FILE* d_file = null;
    unsigned char* buff = null;
    fpos_t temp_fpos = 0;
    size_t s_file_size = 0;
    const size_t max_buff_size = 1024 * 1024 * 3;
    size_t buff_size = 0;
    size_t read_bytes = 0;
    size_t written_bytes = 0;
    size_t i = 0;
    size_t j = 0;
    size_t size_of_enc_key = 0;
    size_t total_bytes_encrypted = 0;

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
    s_file = fopen(source_file, "rb");
    d_file = fopen(dest_file, "wb");

    if(s_file == null || d_file == null) {
        *errMsg = "Access is denied";
        return BV_OP_FAILED;
    }

    size_of_enc_key = strlen(enc_key);
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
    j = 0;
    total_bytes_encrypted = 0;
    while (!feof(s_file)) {
        read_bytes = fread(buff, 1, buff_size, s_file);
        for(i = 0; i<read_bytes; i++) {
            buff[i] = buff[i] ^ ((unsigned char*)enc_key)[j];
            j++;
            if(j == size_of_enc_key) {
                j = 0;
            }
        }
        written_bytes = 0;
        while(written_bytes < read_bytes) {
            written_bytes += fwrite(buff + written_bytes, 1, read_bytes - written_bytes, d_file);
        }
        total_bytes_encrypted += read_bytes;
        callback(code, total_bytes_encrypted, s_file_size);
    }
    fclose(s_file);
    fflush(d_file);
    fclose(d_file);

    free(buff);
    return BV_OP_SUCCESS;
}

int bv_decrypt_file(const char* source_file, const char* dest_file, const char* enc_key, int code, void (*callback)(int code, size_t bytes_decrypted, size_t total_bytes), char** errMsg) {
    return bv_encrypt_file(source_file, dest_file, enc_key, code, callback, errMsg);
}

/**
 * Here if we encrypt some text and if there is a '\0' char in the encrypted text, then
 * the encrypted text will not be treated as full c-string.
 * So if we replace '\0' char with '\xff' char in the enrypted text, then it will remove this problem.
 * Actually normally this solution assumes that we are using only characters as text char in range 0 to 127.
 * So '~' logical operator returns char in range 128 to 255 for the chars of 0 to 127 range.
 *
 * This solution is only valid if:-
 * Text characters and encryption key chars in range 1 to 127(Not 0 to 127 i.e. not including NULL character) i.e the chars are :
 *      \t \n \r [SPACEBAR] ! " # $ % & ' ( ) * + , - . / 0-9 : 58 ; < = > ? @ A-Z [ \ ] ^ _ ` a-z { | } ~
 *
 * @param text
 * @param dest
 */
void bv_encrypt_text(const char* text, const char* key, char* dest) {
    size_t text_ln = strlen(text);
    size_t key_ln = strlen(key);
    if (key_ln == 0) {
        memcpy(dest, text, text_ln + 1);
        return;
    }
    size_t i = 0;
    size_t j = 0;
    for(; i<text_ln; i++, j++) {
        if (j == key_ln)
            j = 0;
        dest[i] = text[i] ^ key[j];
        if (dest[i] == '\0') {
            dest[i] = (char)'\xff';
        }
    }
    dest[i] = '\0';
}

void bv_decrypt_text(const char* text, const char* key, char* dest) {
    size_t text_ln = strlen(text);
    size_t key_ln = strlen(key);
    if (key_ln == 0) {
        memcpy(dest, text, text_ln + 1);
        return;
    }
    size_t i = 0;
    size_t j = 0;
    for(; i<text_ln; i++, j++) {
        if (j == key_ln)
            j = 0;
        if (text[i] == '\xff') {
            dest[i] = (char)'\0' ^ key[j];
        } else {
            dest[i] = text[i] ^ key[j];
        }
    }
    dest[i] = '\0';
}































