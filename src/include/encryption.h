//
// Created by Ariyan Khan on 05-10-2016.
//

#ifndef BYTEVAULT_ENCRYPTION_H
#define BYTEVAULT_ENCRYPTION_H

#include "common.h"

extern const char* const BV_PRIMARY_KEY;
void bv_encrypt_byte_block(const void* target, size_t target_size, const void* key, size_t key_size, void* dest);
void bv_decrypt_byte_block(const void* target, size_t target_size, const void* key, size_t key_size, void* dest);
void bv_encrypt_text(const char* text, const char* key, char* dest);
void bv_decrypt_text(const char* text, const char* key, char* dest);
int bv_encrypt_file(const char* source_file, const char* dest_file, const char* enc_key, int code, void (*callback)(int code, size_t bytes_encrypted, size_t total_bytes), char** errMsg);
int bv_decrypt_file(const char* source_file, const char* dest_file, const char* enc_key, int code, void (*callback)(int code, size_t bytes_decrypted, size_t total_bytes), char** errMsg);

#endif //BYTEVAULT_ENCRYPTION_H


























