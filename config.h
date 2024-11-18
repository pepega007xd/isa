// File: config.h
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#ifndef CONFIG_H
#define CONFIG_H

#include "imports.h"

typedef struct {
        struct sockaddr_in address; /// server address + port of the IMAP server
        char *address_string;       /// server address, as provided by the user
        bool use_tls;               /// whether to use TLS for connecting to the server
        char *certfile;             /// TLS certificate file to use
        char *certaddr;             /// directory containing the TLS certificates to use
        bool only_new;              /// download only new messages
        bool only_headers;          /// download only message headers
        char username[128];         /// username from auth_file
        char password[128];         /// password from auth_file
        char *mailbox;              /// mailbox name to use
        char *out_dir;              /// directory for saving downloaded messages
} Config;

/// parses command line arguments into Config,
/// prints help and exits if there are invalid arguments
Config parse_args(i32 argc, char **argv);

#endif // !CONFIG_H
