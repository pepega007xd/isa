#ifndef CONNECTION_H
#define CONNECTION_H

#include "imports.h"

typedef struct {
        i32 socket_fd; // socket fd for TCP authentication
        SSL_CTX *ssl_ctx;
        SSL *ssl;

} Connection;

/// creates socket, connects to server and establishes TLS connection if required
Connection make_connection(void);

/// `read` abstraction over plain or TLS connection
i64 imap_read(u8 *buffer, u64 length);

/// `write` abstraction over plain or TLS connection with string formatting
i64 imap_write_fmt(const char *format, ...);

#endif // !CONNECTION_H
