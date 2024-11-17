#ifndef SGC_H
#define SGC_H

#include <stdlib.h>

/// Custom wrapper to memory allocator, allows me to deallocate
/// all allocated memory before exit,
///
/// allocation functions are overwritten using macros,
/// must be included before anything else in `imports.h`

/// Works just like malloc.
void *sgc_malloc(size_t size);

/// Works just like calloc.
void *sgc_calloc(size_t nmemb, size_t size);

/// Works just like realloc.
void *sgc_realloc(void *ptr, size_t size);

/// works just like free.
void sgc_free(void *ptr);

/// Deallocates everything allocated to this point,
/// including internal data structures of this allocator.
///
/// Returns the number of freed allocations.
unsigned sgc_free_all(void);

// overwriting the original allocation functions
// so that they are not used directly
#define malloc(size) sgc_malloc(size)
#define realloc(ptr, new_size) sgc_realloc(ptr, new_size)
#define calloc(nmemb, size) sgc_calloc(nmemb, size)
#define free(ptr) sgc_free(ptr)

#endif // !SGC_H
