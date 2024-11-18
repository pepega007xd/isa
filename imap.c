// File: imap.c
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#include "config.h"
#include "connection.h"
#include "imports.h"
#include "utils.h"
#include "vec.h"

#define protocol_error() print_exit("%s:%d: Server returned invalid data", __FILE__, __LINE__);

#define assert_not_null(ptr)                                                                       \
    if (ptr == NULL) {                                                                             \
        protocol_error();                                                                          \
    }

extern Config config;

/// parses out the string literal length from the `{<length>}` specifier at the end of the line
u64 get_string_literal_length(u8Vec line) {
    for (u64 i = line.size - 1; i > 0; i--) {
        if (line.content[i] == '{') {
            return strtoul((char *)line.content + i + 1, NULL, 10);
        }
    }
    print_exit("Could not parse string literal length from '%s'", line.content);
}

/// reads lines from socket, returns the first line which has the provided prefix,
/// fails when tagged response is received
u8Vec wait_for_untagged_response(char *prefix, char *error) {
    while (true) {
        u8Vec line = read_line();

        if (starts_with(line, "TAG")) {
            print_exit(error);
        }

        if (starts_with_nocase(line, prefix)) {
            return line;
        }

        u8_vec_clear(&line);
    }
}

/// reads lines from socket until a fetch response is found
u8Vec wait_for_fetch(void) {
    while (true) {
        u8Vec line = read_line();

        if (starts_with(line, "TAG")) {
            print_exit("Waiting for fetch response failed");
        }

        if (starts_with(line, "*")) {
            // skip two tokens "*" and sequence number
            strtok((char *)line.content, " ");
            strtok(NULL, " ");
            char *keyword = strtok(NULL, " ");

            assert_not_null(keyword);

            if (string_eq_nocase(keyword, "FETCH")) {
                return line;
            }
        }

        u8_vec_clear(&line);
    }
}

/// reads lines from socket until a tagged response is found,
/// fails if the response is NO or BAD
void wait_for_ok_response(char *error) {
    while (true) {
        u8Vec line = read_line();
        if (starts_with(line, "TAG")) {
            strtok((char *)line.content, " ");
            char *response = strtok(NULL, " ");
            assert_not_null(response);

            if (string_eq_nocase(response, "OK")) {
                u8_vec_clear(&line);
                return;
            } else {
                print_exit(error);
            }
        }

        u8_vec_clear(&line);
    }
}

/// parses out the UIDVALIDITY number from its response line
u32 parse_uidvalidity(u8Vec uidvalidity_response) {
    // skip three tokens: "*", "OK" and "[UIDVALIDITY"
    strtok((char *)uidvalidity_response.content, " ");
    strtok(NULL, " ");
    strtok(NULL, " ");

    char *uidvalidity = strtok(NULL, " ");
    assert_not_null(uidvalidity);

    return (u32)strtoul(uidvalidity, NULL, 10);
}

/// sends UID FETCH command for a given `uid` and saves received message content
/// as-is into a file at a location given by `filename`
void download_message(u32 uid, char *filename) {
    if (config.only_headers) {
        imap_write_fmt("TAG%d UID FETCH %u BODY.PEEK[HEADER]", get_tag(), uid);
    } else {
        imap_write_fmt("TAG%d UID FETCH %u BODY[]", get_tag(), uid);
    }

    u8Vec fetch_line = wait_for_fetch();
    // fetch line will contain `{<length>}` at the end
    u64 length = get_string_literal_length(fetch_line);
    u8_vec_clear(&fetch_line);

    u8 *message_buffer = malloc(length);
    imap_read(message_buffer, length);

    FILE *message_file = fopen(filename, "wb");
    if (message_file == NULL) {
        print_exit("Failed to open file '%s': %s", filename, strerror(errno));
    }

    fwrite(message_buffer, 1, length, message_file);

    fclose(message_file);
    free(message_buffer);

    wait_for_ok_response("Could not download message");
}

/// parses out UID numbers from a SEARCH response
u32Vec parse_uids(char *search_response) {
    char *delimiters = " \r\n";

    // skip two tokens, which will be "*" and "SEARCH"
    strtok(search_response, delimiters);
    strtok(NULL, delimiters);

    // read numbers from input
    char *token = strtok(NULL, delimiters);
    u32Vec uids = u32_vec_new();
    while (token != NULL) {
        u32_vec_push(&uids, (u32)strtoul(token, NULL, 10)); // UIDs are 32-bit
        token = strtok(NULL, delimiters);
    }

    return uids;
}

/// returns an allocated buffer with the formatted filename, which contains:
///
/// - server address
/// - mailbox name
/// - whether the file contains only headers
/// - UIDVALIDITY and UID numbers
///
/// This filename uniquely identifies a given message
char *get_filename(u32 uidvalidity, u32 uid) {
    i32 buffer_size =
        snprintf(NULL, 0, "%s/%s-%s%s-%u-%u.eml", config.out_dir, config.address_string,
                 config.mailbox, config.only_headers ? "-h" : "", uidvalidity, uid);
    if (buffer_size == -1) {
        print_exit("failed to format filename");
    }
    buffer_size++;

    char *buffer = malloc((u64)buffer_size);
    sprintf(buffer, "%s/%s-%s%s-%u-%u.eml", config.out_dir, config.address_string, config.mailbox,
            config.only_headers ? "-h" : "", uidvalidity, uid);
    buffer[buffer_size - 1] = '\0';

    return buffer;
}

/// handles the whole process of downloading messages
void download_messages(void) {
    // login to the server
    imap_write_fmt("TAG%d LOGIN %s %s", get_tag(), config.username, config.password);
    wait_for_ok_response("Could not log in");

    // select the mailbox
    imap_write_fmt("TAG%d SELECT %s", get_tag(), config.mailbox);
    u8Vec uidvalidity_response =
        wait_for_untagged_response("* OK [UIDVALIDITY", "Could not select mailbox");
    u32 uidvalidity = parse_uidvalidity(uidvalidity_response);
    u8_vec_clear(&uidvalidity_response);
    wait_for_ok_response("Could not select mailbox");

    // get message UIDs
    if (config.only_new) {
        imap_write_fmt("TAG%d UID SEARCH UNSEEN", get_tag(), config.mailbox);
    } else {
        imap_write_fmt("TAG%d UID SEARCH ALL", get_tag(), config.mailbox);
    }
    u8Vec search_response = wait_for_untagged_response("* SEARCH", "Could not list messages");
    wait_for_ok_response("Could not list messages");

    // parse message UIDs
    u32Vec uids = parse_uids((char *)search_response.content);
    u8_vec_clear(&search_response);

    // download messages
    u32 downloaded = 0;
    for (u64 i = 0; i < uids.size; i++) {
        u32 uid = uids.content[i];
        char *filename = get_filename(uidvalidity, uid);

        if (!file_exists(filename)) {
            download_message(uid, filename);
            downloaded++;
        }

        free(filename);
    }
    u32_vec_clear(&uids);

    // print message to the user
    printf("Downloaded %u%s %s from %s\n", downloaded, config.only_new ? " new" : "",
           config.only_headers ? "headers" : "messages", config.mailbox);

    // logout of the server
    imap_write_fmt("TAG%d LOGOUT", get_tag());
    wait_for_ok_response("Could not log out");
}
