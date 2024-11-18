// File: main.c
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#include "config.h"
#include "connection.h"
#include "imap.h"
#include "imports.h"

/// program configuration, set by `parse_args`, then read-only
Config config = {0};

/// struct containing socket and openssl data representing an active connection,
/// set by `make_connection`, then read-only
Connection connection = {0};

int main(i32 argc, char **argv) {
    // parse program arguments
    config = parse_args(argc, argv);

    // open socket, initialize TLS, connect to server
    connection = make_connection();

    download_messages();

    // close connection, free all memory
    free_all_resources();
    exit(EXIT_SUCCESS);
}
