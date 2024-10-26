#include "config.h"
#include "connection.h"
#include "imap.h"
#include "imports.h"
#include "utils.h"

/// program configuration, set by parse_args, then read-only
Config config;

/// struct containing socket and openssl data representing an active connection
Connection connection;

int main(i32 argc, char **argv) {
    // parse program arguments
    config = parse_args(argc, argv);

    // open socket, establish connection
    connection = make_connection();

    download_messages();

    free_all_resources();
    exit(EXIT_SUCCESS);
}
