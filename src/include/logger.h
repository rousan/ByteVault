//
// Created by Rousan Ali on 05-10-2016.
//

#ifndef BYTEVAULT_LOGGER_H
#define BYTEVAULT_LOGGER_H

#include "common.h"

#define debug (printf(" [%lld] ", __LINE__))

void bv_log(const char* format, ...);
void bv_warn(const char* format, ...);
void bv_error(const char* format, ...);

#endif //BYTEVAULT_LOGGER_H
