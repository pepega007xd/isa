#include "connection.h"
#include "config.h"
#include "utils.h"

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
        if ((connection.ssl = SSL_new(connection.ssl_ctx)) == NULL) {
            print_exit("Could not create SSL object");
        }

        // associate socket with SSL object
        if (SSL_set_fd(connection.ssl, connection.socket_fd) != 1) {
            print_exit("Could not establish TLS connection");
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
    if (config.use_tls) {
        return SSL_read(connection.ssl, buffer, (i32)length);
    } else {
        return read(connection.socket_fd, buffer, length);
    }
}

i64 imap_write_fmt(const char *format, ...) {
    va_list args, args_copy;
    va_start(args, format);
    va_copy(args_copy, args);

    i32 buffer_len = vsnprintf(NULL, 0, format, args);
    assert(buffer_len >= 0); // negative size means error

    char *buffer = malloc((u64)(buffer_len + 1));
    buffer[buffer_len - 1] = '\0';

    vsprintf(buffer, format, args_copy);

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
