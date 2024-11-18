// File: config.c
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#include "config.h"
#include "imports.h"
#include "utils.h"

#define print_usage_exit()                                                                         \
    print_exit("Usage: imapcl server [-p port] [-T [-c certfile] [-C certaddr]] "                  \
               "[-n] [-h] -a auth_file [-b MAILBOX] -o out_dir");

bool parse_auth_file(char *auth_file, Config *config) {
    FILE *auth = fopen(auth_file, "r");
    if (auth == NULL) {
        return false;
    }

    i32 matched =
        fscanf(auth, "username = %127s\npassword = %127s", config->username, config->password);

    fclose(auth);
    return matched == 2;
}

Config parse_args(i32 argc, char **argv) {
    // all other fields are initialized to zero
    Config config = {
        .certaddr = "/etc/ssl/certs/",
        .mailbox = "INBOX",
    };

    if (argc < 2) {
        print_usage_exit();
    }

    char *auth_file = NULL;
    char *port = NULL;
    i32 opt;

    while (optind < argc) {
        while ((opt = getopt(argc, argv, "p:Tc:C:nha:b:o:")) != -1) {
            switch (opt) {
            case 'p':
                port = optarg;
                break;

            case 'T':
                config.use_tls = true;
                break;

            case 'c':
                config.certfile = optarg;
                break;

            case 'C':
                config.certaddr = optarg;
                break;

            case 'n':
                config.only_new = true;
                break;

            case 'h':
                config.only_headers = true;
                break;

            case 'a':
                auth_file = optarg;
                break;

            case 'b':
                config.mailbox = optarg;
                break;

            case 'o':
                config.out_dir = optarg;
                break;

            default:
                print_usage_exit();
            }
        }

        // read server address
        if (optind < argc) {
            config.address_string = argv[optind];
            optind++;
        }
    }

    // check if all required arguments were provided
    if (config.address_string == NULL || auth_file == NULL || config.out_dir == NULL) {
        print_usage_exit();
    }

    if (!parse_auth_file(auth_file, &config)) {
        print_exit("Could not read auth file '%s'", auth_file);
    }

    // set default port if there is none explicitly provided
    if (config.use_tls && port == NULL) {
        port = "993";
    } else if (port == NULL) {
        port = "143";
    }

    // convert text address/hostname into `struct sockaddr_in`
    struct addrinfo *result = NULL;
    struct addrinfo hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};

    i32 error = getaddrinfo(config.address_string, port, &hints, &result);

    if (error != 0 || result == NULL) {
        print_exit("Cannot resolve address %s port %s: %s", config.address_string, port,
                   gai_strerror(error));
    }

    config.address = *(struct sockaddr_in *)result->ai_addr;

    freeaddrinfo(result);

    return config;
}
