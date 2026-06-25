#include <mem.h>
#include <string.h>

void* alloc_copy(int32_t size, const void* src) {
    void* chunk = malloc(size);

    if (chunk == NULL) {
        return NULL;
    }

    memcpy(chunk, src, size);

    return chunk;
}

void* str_alloc_copy(const void* src) {
    int32_t length = strlen(src) + 1; // `+ 1` accounts for null terminator
    void* chunk = malloc(length);

    if (chunk == NULL) {
        return NULL;
    }

    memcpy(chunk, src, length);

    return chunk;
}
