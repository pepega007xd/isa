#ifndef VEC_H
#define VEC_H

#include "imports.h"
#include "utils.h"

/// generates a vector data structure for a given type, including functions for
/// using that vector

/// generates the declarations of the type, and functions,
///
/// `Type` is in UpperCamelCase, used for generating type name, `type` is in
/// snake_case, used for generating associated functions
#define GEN_VEC_H(Type, type)                                                  \
    typedef struct {                                                           \
            u64 size;                                                          \
            u64 capacity;                                                      \
            Type *content;                                                     \
    } Type##Vec;                                                               \
                                                                               \
    Type##Vec type##_vec_new(void);                                            \
    Type##Vec type##_vec_reserve(u64 size);                                    \
    Type *type##_vec_push(Type##Vec *vec, Type value);                         \
    Type type##_vec_pop(Type##Vec *vec);                                       \
    void type##_vec_append(Type##Vec *vec, Type *values, u64 count);           \
    void type##_vec_delete(Type##Vec *vec, u64 index);                         \
    void type##_vec_clear(Type##Vec *vec);

/// generates the implementation of associated functions
#define GEN_VEC(Type, type)                                                    \
    Type##Vec type##_vec_new(void) {                                           \
        return (Type##Vec){.size = 0, .capacity = 0, .content = NULL};         \
    }                                                                          \
    Type##Vec type##_vec_reserve(u64 size) {                                   \
        return (Type##Vec){.content = malloc(sizeof(Type) * size),             \
                           .size = 0,                                          \
                           .capacity = size};                                  \
    }                                                                          \
                                                                               \
    Type *type##_vec_push(Type##Vec *vec, Type value) {                        \
        if (vec->size == vec->capacity) {                                      \
            u64 new_cap = vec->capacity * 2 + 1;                               \
            vec->content = realloc(vec->content, new_cap * sizeof(Type));      \
            vec->capacity = new_cap;                                           \
        }                                                                      \
        vec->content[vec->size++] = value;                                     \
        return &vec->content[vec->size - 1];                                   \
    }                                                                          \
                                                                               \
    Type type##_vec_pop(Type##Vec *vec) {                                      \
        if (vec->size == 0) {                                                  \
            print_exit("pop from empty vec");                                  \
        }                                                                      \
                                                                               \
        return vec->content[vec->size--];                                      \
    }                                                                          \
                                                                               \
    void type##_vec_append(Type##Vec *vec, Type *values, u64 count) {          \
        vec->capacity += count;                                                \
        vec->content = realloc(vec->content, vec->capacity * sizeof(Type));    \
        memcpy(vec->content + vec->size, values, count * sizeof(Type));        \
        vec->size += count;                                                    \
    }                                                                          \
                                                                               \
    void type##_vec_delete(Type##Vec *vec, u64 index) {                        \
        for (u64 i = index; i < vec->size - 1; i++) {                          \
            vec->content[i] = vec->content[i + 1];                             \
        }                                                                      \
        vec->size--;                                                           \
    }                                                                          \
                                                                               \
    void type##_vec_clear(Type##Vec *vec) {                                    \
        free(vec->content);                                                    \
        vec->content = NULL;                                                   \
        vec->size = 0;                                                         \
        vec->capacity = 0;                                                     \
    }

// the following can be generated only for primitive types
// (comparable with == operator)
#define GEN_VEC_COMPARE_H(Type, type)                                          \
    /* Returns the index of first found element, or -1 if not found */         \
    i32 type##_vec_find(Type##Vec vec, Type value, i32 start);

#define GEN_VEC_COMPARE(Type, type)                                            \
    i32 type##_vec_find(Type##Vec vec, Type value, i32 start) {                \
        if (start < 0) {                                                       \
            return -1;                                                         \
        };                                                                     \
                                                                               \
        while ((u64)start < vec.size) {                                        \
            if (vec.content[start] == value) {                                 \
                return start;                                                  \
            }                                                                  \
            start++;                                                           \
        }                                                                      \
        return -1;                                                             \
    }

GEN_VEC_H(u8, u8)
GEN_VEC_COMPARE_H(u8, u8)

GEN_VEC_H(u16, u16)
GEN_VEC_COMPARE_H(u16, u16)

/// appends a null-terminated string to a vector of bytes
void append_string(u8Vec *vector, char *string);

#endif //! VEC_H
