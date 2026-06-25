#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stdlib.h>

/* Allocates a chunk of memory and copies right after */
void* alloc_copy(int32_t size, const void* src);

/* Ditto, but relies on null terminator of the string */
void* str_alloc_copy(const void* src);

#endif
