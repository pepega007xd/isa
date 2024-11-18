// File: imports.h
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#ifndef IMPORTS_H
#define IMPORTS_H

// import allocator before anything else
#include "sgc.h"
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

// better integer types

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// debugging macros

#define here(...)                                                                                  \
    {                                                                                              \
        fprintf(stderr, "HERE: ");                                                                 \
        fprintf(stderr, __VA_ARGS__);                                                              \
        fprintf(stderr, "\n");                                                                     \
    }

#define trace() *(char *)0 = 0;

#endif // !IMPORTS_H
