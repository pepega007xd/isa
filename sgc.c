// File: sgc.c
// Author: Tomáš Brablec (xbrabl04)
// Date: 2024-11-18

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

typedef struct {
        void **allocs;
        size_t size;
        size_t capacity;
} AllocTable;

AllocTable alloc_table = {.allocs = NULL, .size = 0, .capacity = 0};

size_t sgc_free_all(void) {
    size_t orig_size = alloc_table.size;

    for (size_t i = 0; i < alloc_table.size; i++) {
        free(alloc_table.allocs[i]);
    }

    free(alloc_table.allocs);
    alloc_table.allocs = NULL;
    alloc_table.size = 0;
    alloc_table.capacity = 0;
    return orig_size;
}

noreturn void err_free_exit(void) {
    sgc_free_all();
    fprintf(stderr, "Memory allocation failed.\n");
    exit(99);
}

void *sgc_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL)
        err_free_exit();

    // resize allocation table if needed
    if (alloc_table.size == alloc_table.capacity) {
        size_t new_cap = alloc_table.capacity * 2 + 1;
        alloc_table.allocs = realloc(alloc_table.allocs, sizeof(void *) * new_cap);
        if (alloc_table.allocs == NULL)
            err_free_exit();
        alloc_table.capacity = new_cap;
    }

    alloc_table.allocs[alloc_table.size++] = ptr;

    return ptr;
}

void *sgc_calloc(size_t nmemb, size_t size) {
    void *ptr = sgc_malloc(size * nmemb);
    memset(ptr, 0, size * nmemb);
    return ptr;
}

void *sgc_realloc(void *ptr, size_t size) {
    // if original pointer is NULL, realloc behaves like malloc
    if (ptr == NULL) {
        return sgc_malloc(size);
    }

    for (size_t i = 0; i < alloc_table.size; i++) {
        if (alloc_table.allocs[i] == ptr) {
            alloc_table.allocs[i] = realloc(ptr, size);
            return alloc_table.allocs[i];
        }
    }

    fprintf(stderr, "Invalid realloc on address %p (new size %zu)\n", ptr, size);
    err_free_exit();
}

void sgc_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    size_t delete_pos = (size_t)-1;
    for (size_t i = 0; i < alloc_table.size; i++) {
        if (alloc_table.allocs[i] == ptr) {
            free(ptr);
            delete_pos = i;
            break;
        }
    }

    if (delete_pos == (size_t)-1) {
        fprintf(stderr, "Invalid free on address %p\n", ptr);
        err_free_exit();
    }

    for (size_t i = delete_pos; i < alloc_table.size - 1; i++)
        alloc_table.allocs[i] = alloc_table.allocs[i + 1];

    alloc_table.size--;
}
