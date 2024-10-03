#include "vec.h"
#include "imports.h"

GEN_VEC(u8, u8)
GEN_VEC_COMPARE(u8, u8)

GEN_VEC(u16, u16)
GEN_VEC_COMPARE(u16, u16)

void append_string(u8Vec *vector, char *string) {
    u8 c;
    while ((c = (u8)*string) != 0) {
        u8_vec_push(vector, c);
        string++;
    }
}
