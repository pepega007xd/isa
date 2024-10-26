#include "config.h"
#include "connection.h"
#include "imports.h"
#include "utils.h"
#include "vec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define protocol_error() print_exit("Server returned invalid data");

extern Config config;

u32 get_tag(void) {
    static u32 tag;
    return tag++;
}

u8Vec read_line(void) {
    u8Vec message_buffer = u8_vec_new();
    // read bytes from socket up to '\n' - single message
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

void debug_print_all(void) {
    while (true) {
        u8Vec line = read_line();
        here("%s", line.content);
    }
}

bool starts_with(u8Vec data, char *prefix) {
    u64 prefix_len = strlen(prefix);
    if (data.size < prefix_len) {
        return false;
    }

    for (u64 i = 0; i < prefix_len; i++) {
        if (data.content[i] != prefix[i]) {
            return false;
        }
    }

    return true;
}

u64 get_message_length(u8Vec fetch_line) {
    for (u64 i = fetch_line.size - 1; i > 0; i--) {
        if (fetch_line.content[i] == '{') {
            return strtoul((char *)fetch_line.content + i + 1, NULL, 10);
        }
    }

    print_exit("Did not find any data in message"); // TODO: error handling
}

u8Vec wait_for_untagged_response(char *prefix) {
    while (true) {
        u8Vec line = read_line();

        if (starts_with(line, "TAG")) {
            print_exit("Waiting for untagged response '%s' failed", prefix);
        }

        if (starts_with(line, prefix)) {
            return line;
        }

        u8_vec_clear(&line);
    }
}

u8Vec wait_for_fetch(void) {
    while (true) {
        u8Vec line = read_line();

        if (starts_with(line, "TAG")) {
            here("%s", line.content);
            print_exit("Waiting for fetch response failed");
        }

        if (starts_with(line, "*")) {
            // skip two tokens "*" and sequence number
            strtok((char *)line.content, " ");
            strtok(NULL, " ");
            char *keyword = strtok(NULL, " ");

            if (keyword == NULL) {
                protocol_error();
            }

            if (string_eq_nocase(keyword, "FETCH")) {
                return line;
            }
        }
        u8_vec_clear(&line);
    }
}

void assert_tagged_response_ok(void) {
    while (true) {
        u8Vec line = read_line();
        if (starts_with(line, "TAG")) {
            strtok((char *)line.content, " ");
            char *response = strtok(NULL, " ");
            if (response == NULL) {
                protocol_error();
            }

            if (string_eq(response, "OK")) {
                u8_vec_clear(&line);
                return;
            } else {
                print_exit("Server returned NO or BAD"); // TODO better error message
            }
        }
        u8_vec_clear(&line);
    }
}

void download_message(u32 uid, char *filename) {
    if (config.only_headers) {
        imap_write_fmt("TAG%d UID FETCH %u BODY.PEEK[HEADER]", get_tag(), uid);
    } else {
        imap_write_fmt("TAG%d UID FETCH %u BODY[]", get_tag(), uid);
    }

    u8Vec fetch_line = wait_for_fetch();

    u64 length = get_message_length(fetch_line);
    u8 *message_buffer = malloc(length);
    imap_read(message_buffer, length);

    FILE *message_file = fopen(filename, "wb");
    if (message_file == NULL) {
        print_exit("Failed to open file '%s': %s", filename, strerror(errno));
    }

    fwrite(message_buffer, length, 1, message_file);

    fclose(message_file);
    free(message_buffer);

    assert_tagged_response_ok();
}

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

u32 parse_uidvalidity(u8Vec uidvalidity_response) {
    // skip three tokens: "*", "OK" and "[UIDVALIDITY"
    strtok((char *)uidvalidity_response.content, " ");
    strtok(NULL, " ");
    strtok(NULL, " ");

    char *uidvalidity = strtok(NULL, " ");
    if (uidvalidity == NULL) {
        protocol_error();
    }
    return (u32)strtoul(uidvalidity, NULL, 10);
}

char *get_filename(u32 uidvalidity, u32 uid) {
    char *filename;
    i32 result = asprintf(&filename, "%s/%s-%s-%u-%u.eml", config.out_dir, config.address_string,
                          config.mailbox, uidvalidity, uid);
    if (result == -1) {
        print_exit("failed to format filename");
    }

    return filename;
}

bool file_exists(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return false;
    }

    fclose(file);
    return true;
}

void download_messages(void) {
    // log into the server
    imap_write_fmt("TAG%d LOGIN %s %s", get_tag(), config.username, config.password);
    assert_tagged_response_ok();

    // select the mailbox
    imap_write_fmt("TAG%d SELECT %s", get_tag(), config.mailbox);
    u8Vec uidvalidity_response = wait_for_untagged_response("* OK [UIDVALIDITY");
    u32 uidvalidity = parse_uidvalidity(uidvalidity_response);
    u8_vec_clear(&uidvalidity_response);
    assert_tagged_response_ok();

    // get message UIDs
    if (config.only_new) {
        imap_write_fmt("TAG%d UID SEARCH NEW", get_tag(), config.mailbox);
    } else {
        imap_write_fmt("TAG%d UID SEARCH ALL", get_tag(), config.mailbox);
    }
    u8Vec search_response = wait_for_untagged_response("* SEARCH");
    assert_tagged_response_ok();

    // parse message UIDs
    u32Vec uids = parse_uids((char *)search_response.content);
    u8_vec_clear(&search_response);

    // download messages
    for (u64 i = 0; i < uids.size; i++) {
        u32 uid = uids.content[i];
        char *filename = get_filename(uidvalidity, uid);
        if (!file_exists(filename)) {
            download_message(uid, filename);
        }

        // get_filename allocates with original malloc - use original free
        original_free(filename);
    }
    u32_vec_clear(&uids);

    imap_write_fmt("TAG%d LOGOUT", get_tag());

    assert_tagged_response_ok();
}
