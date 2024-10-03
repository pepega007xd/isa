#include "connection.h"
#include "config.h"
#include "utils.h"

extern Config config;

Connection make_connection(void) {
    Connection connection = {};

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

i64 imap_write(u8 *buffer, u64 length) {
    if (config.use_tls) {
        return SSL_write(connection.ssl, buffer, (i32)length);
    } else {
        return write(connection.socket_fd, buffer, length);
    }
}
