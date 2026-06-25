#include <assert.h>
#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_node_pool(NodePool* npool, size_t size) {
    npool->data = calloc(size, 1);
    npool->count = 0;
    npool->capacity = size;
}

void free_node_pool(NodePool* npool) {
    free(npool->data);
    npool->data = NULL;
}

void* allocate_node_pool(NodePool* npool, size_t size) {
    // TODO: Determine CPU's default byte alignment size, for now it's 8 here
    size_t aligned_size = (size + 7) & ~7;
    // printf("original size: %ld; allocated size: %ld\n", size,aligned_size);

    if (npool->count + aligned_size >= npool->capacity) {
        printf("NodePool reached max size (%ld > %ld), time to implement resizing\n", npool->count + aligned_size, npool->capacity);
        assert(false);
    }

    void* ptr = &((char*)(npool->data))[npool->count];
    npool->count += aligned_size;

    return ptr;
}

void* allocate_and_write_node_pool(NodePool* npool, const void* data, size_t size) {
    void* ptr = allocate_node_pool(npool, size);
    memcpy(ptr, data, size);

    return ptr;
}
