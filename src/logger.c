//
// Created by Rousan Ali on 05-10-2016.
//

#include <stdarg.h>

#include "include/logger.h"
#include "include/utils.h"
#include "include/utils.h"
#include "include/viewer.h"

void bv_log(const char* format, ...) {
    va_list vaList;

    va_start(vaList, format);
    bv_resolve_str(&format);
    bv_console_text_color(BV_LOG_COLOR);
    fprintf(stdout, "Log:");
    bv_console_text_color(BV_NORMAL_COLOR);
    fprintf(stdout, " ");
    vfprintf(stdout, format, vaList);
    fprintf(stdout, "\n");
}


void bv_warn(const char* format, ...) {
    va_list vaList;

    va_start(vaList, format);
    bv_resolve_str(&format);
    bv_console_text_color(BV_WARNING_COLOR);
    fprintf(stderr, "Warning:");
    bv_console_text_color(BV_NORMAL_COLOR);
    fprintf(stderr, " ");
    vfprintf(stderr, format, vaList);
    fprintf(stderr, "\n");
}

void bv_error(const char* format, ...) {
    va_list vaList;

    va_start(vaList, format);
    bv_resolve_str(&format);
    bv_console_text_color(BV_ERROR_COLOR);
    fprintf(stderr, "Error:");
    bv_console_text_color(BV_NORMAL_COLOR);
    fprintf(stderr, " ");
    vfprintf(stderr, format, vaList);
    fprintf(stderr, "\n");
}



















