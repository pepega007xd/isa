// File: vec.c
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#include "vec.h"
#include "imports.h"

GEN_VEC(u8, u8)
GEN_VEC_COMPARE(u8, u8)

GEN_VEC(u32, u32)
GEN_VEC_COMPARE(u32, u32)

void append_string(u8Vec *vector, char *string) {
    u8 c;
    while ((c = (u8)*string) != 0) {
        u8_vec_push(vector, c);
        string++;
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

bool starts_with_nocase(u8Vec data, char *prefix) {
    u64 prefix_len = strlen(prefix);
    if (data.size < prefix_len) {
        return false;
    }

    for (u64 i = 0; i < prefix_len; i++) {
        if (toupper(data.content[i]) != toupper(prefix[i])) {
            return false;
        }
    }

    return true;
}
