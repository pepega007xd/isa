// File: connection.c
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#include "connection.h"
#include "config.h"
#include "imports.h"
#include "utils.h"
#include "vec.h"

extern Config config;

Connection make_connection(void) {
    Connection connection = {0};

    connection.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connection.socket_fd == -1) {
        print_exit("Failed to create socket: %s", strerror(errno));
    }

    if (connect(connection.socket_fd, (struct sockaddr *)&config.address, sizeof(config.address)) ==
        -1) {
        print_exit("Could not connect to server: %s", strerror(errno));
    }

    if (config.use_tls) {
        connection.ssl_ctx = SSL_CTX_new(TLS_client_method());

        if (SSL_CTX_load_verify_locations(connection.ssl_ctx, config.certfile, config.certaddr) !=
            1) {
            print_exit("Could not set specified TLS certificate file/path");
        }

        // turn on server verification
        SSL_CTX_set_verify(connection.ssl_ctx, SSL_VERIFY_PEER, NULL);

        if ((connection.ssl = SSL_new(connection.ssl_ctx)) == NULL) {
            print_exit("Could not create SSL object");
        }

        // associate socket with SSL object
        if (SSL_set_fd(connection.ssl, connection.socket_fd) != 1) {
            print_exit("Could not create TLS connection");
        }

        // setup TLS connnection
        if (SSL_connect(connection.ssl) != 1) {
            print_exit("Could not establish TLS connection");
        }
    }

    return connection;
}

extern Connection connection;

i64 imap_read(u8 *buffer, u64 length) {
    for (u64 i = 0; i < length; i++) {
        i64 result;
        if (config.use_tls) {
            result = SSL_read(connection.ssl, buffer + i, 1);
        } else {
            result = read(connection.socket_fd, buffer + i, 1);
        }

        if (result != 1) {
            return result;
        }
    }

    return (i64)length;
}

u8Vec read_line(void) {
    u8Vec message_buffer = u8_vec_new();
    // read bytes from socket up to '\n' - single line
    while (true) {
        u8 message_byte;
        i64 size = imap_read(&message_byte, 1);

        if (size == 0) {
            print_exit("IMAP server closed connection");

        } else if (size == -1) {
            print_exit("Failed to receive TCP message: %s", strerror(errno));

        } else {
            u8_vec_push(&message_buffer, message_byte);
            if (message_byte == '\n') {
                break;
            }
        }
    }

    u8_vec_push(&message_buffer, '\0');
    return message_buffer;
}

i64 imap_write_fmt(const char *format, ...) {
    va_list args, args_copy;
    va_start(args, format);
    va_copy(args_copy, args);

    i32 buffer_len = vsnprintf(NULL, 0, format, args);
    assert(buffer_len >= 0); // negative size means error
    buffer_len += 2;         // for CRLF

    char *buffer = malloc((u64)(buffer_len));
    vsprintf(buffer, format, args_copy);
    buffer[buffer_len - 2] = '\r';
    buffer[buffer_len - 1] = '\n';

    i64 result;
    if (config.use_tls) {
        result = SSL_write(connection.ssl, buffer, buffer_len);
    } else {
        result = write(connection.socket_fd, buffer, (u64)buffer_len);
    }

    free(buffer);
    va_end(args_copy);
    va_end(args);
    return result;
}
