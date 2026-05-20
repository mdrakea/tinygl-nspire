/*
 * Memory allocator for TinyGL
 */
#include "zgl.h"
#include <string.h>

#ifndef ARENA_SIZE
#define ARENA_SIZE  (16 * 1024 * 1024)
#endif
#define NUM_BUCKETS 32
#define ALIGN       16

static char arena[ARENA_SIZE];
static size_t arena_off = 0;
static void *free_lists[NUM_BUCKETS];

static inline int bucket_for(size_t size)
{
    int b = 0;
    size_t s = ALIGN;
    while (s < size) { s <<= 1; b++; }
    return b;
}

void *gl_malloc(int size)
{
    if (size <= 0) size = 1;

    /* Reserve room for 1-word header (bucket index) */
    size_t total = (size_t)size + ALIGN;
    int b = bucket_for(total);
    size_t bsize = (size_t)ALIGN << b;

    void *block;
    if (free_lists[b]) {
        block = free_lists[b];
        free_lists[b] = *(void **)block;
    } else {
        block = &arena[arena_off];
        arena_off += bsize;
        /* Optional: check overflow */
    }

    *(int *)block = b;
    return (char *)block + ALIGN;
}

void gl_free(void *p)
{
    if (!p) return;
    void *block = (char *)p - ALIGN;
    int b = *(int *)block;
    *(void **)block = free_lists[b];
    free_lists[b] = block;
}

inline void* gl_zalloc(int size)
{
    void *p = gl_malloc(size);
    memset(p, 0, size);
    return p;
}