// File: utils.h
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#ifndef UTILS_H
#define UTILS_H

#include "imports.h"

#define BUFFER_SIZE 1500

/// Print formatted error message, add newline,
/// free all resources, and exit
noreturn void print_exit(const char *format, ...);

/// closes the connection and frees all resources allocated by the program
void free_all_resources(void);

/// deallocates all memory and terminates the program
noreturn void exit_fail(void);

/// returns true if the two strings are equal,
/// ascii letters are compared case-insensitively
bool string_eq_nocase(const char *lhs, const char *rhs);

/// returns true if `string` is an empty string
bool string_is_empty(const char *string);

/// get current time in milliseconds
i64 get_time_ms(void);

/// returns true when a file with the given filenames exists
bool file_exists(char *filename);

/// returns a unique integer tag on each call
u32 get_tag(void);

#endif // !UTILS_H
