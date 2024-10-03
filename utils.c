#include "connection.h"
#include "imports.h"
#include <stdio.h>

extern Connection connection;

void eprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "ERR: ");
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    va_end(args);
}

void free_all_resources(void) {
    // close the TLS connection
    SSL_shutdown(connection.ssl);

    // deallocate openssl resources
    SSL_free(connection.ssl);
    SSL_CTX_free(connection.ssl_ctx);

    // close the TCP socket
    close(connection.socket_fd);

    // free all memory
    sgc_free_all();
}

noreturn void print_exit(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    fputc('\n', stderr);

    va_end(args);

    free_all_resources();
    exit(EXIT_FAILURE);
}

bool string_eq(const char *lhs, const char *rhs) { return strcmp(lhs, rhs) == 0; }

bool string_eq_nocase(const char *lhs, const char *rhs) {
    u64 lhs_len = strlen(lhs);
    u64 rhs_len = strlen(rhs);
    if (lhs_len != rhs_len) {
        return false;
    }

    for (u64 i = 0; i < lhs_len; i++) {
        if (toupper(lhs[i]) != toupper(rhs[i])) {
            return false;
        }
    }

    return true;
}

bool string_is_empty(const char *string) { return strlen(string) == 0; }
