#pragma once
#include <endian.h>


#ifndef SEGM_SIZE
// 256 bytes max segment size
#define SEGMENT_SIZE (1 << 10)
#endif


#ifndef STACK_THRESHOLD
// Maximum amount of memory that can be used as a temp buffer on the stack
#define STACK_THRESHOLD (1 << 11)
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "debug.h"


typedef struct {
    // Amount of segments
    size_t size;
    // Amount of possible segments
    size_t capacity;

    // Everything is LITTLE endian. This means the smallest
    // the least significant segments come first
    char *segments[];
} BigInt;


static inline BigInt *bigint_new() {
    BigInt *bigint = calloc(1, sizeof(BigInt) + 1024 * sizeof(char *));
    bigint->size = 0;
    bigint->capacity = 4;

    return bigint;
}

static inline BigInt *bigint_with_capacity(size_t capacity) {
    BigInt *bigint = calloc(1, sizeof(BigInt) + capacity * sizeof(char *));
    bigint->size = 0;
    bigint->capacity = capacity;

    return bigint;
}

static inline BigInt *bigint_from(uint64_t value) {
    BigInt *bi = bigint_with_capacity(128);
    bi->size = 1;
    bi->segments[0] = aligned_alloc(32, SEGMENT_SIZE);
    memset(bi->segments[0], 0, SEGMENT_SIZE);
    *(uint64_t *) bi->segments[0] = htole64(value);
    return bi;
}

static inline void bigint_grow_for(BigInt **bigint, size_t new_size) {
    BigInt *curr = *bigint;
    if (new_size <= curr->capacity) return;

    // EXPONENTIAL GROWTH!
    size_t grown_cap = (curr->capacity + 1) << 1;
    curr->capacity = grown_cap > new_size ? grown_cap : new_size;
    curr->capacity = curr->capacity > 1024 ? curr->capacity : 1024;
    debug_assert(curr->capacity != 0);

    *bigint = realloc(*bigint, sizeof(BigInt) + curr->capacity * sizeof(char *));
    debug_assert(*bigint != NULL);
}

static inline void bigint_free(BigInt *bigint) {
    for (size_t i = 0; i < bigint->size; i++) free(bigint->segments[i]);
    free(bigint);
}

char *bigint_hexdump(const BigInt *restrict bigint);

// Place n zero filled segments at the beginning of the bigint.
// Ie: INT << (SEGMENT_SIZE*8*n)
void bigint_segment_shl(BigInt **bigint, uint16_t amount);

// Discard n segments from the beginning of the bigint.
// Ie: INT >> (SEGMENT_SIZE*8*n)
void bigint_segment_shr(BigInt **bigint, uint16_t amount);


// Byte shift left.
// Ie INT << (8*n)
void bigint_byte_shl_memmove(BigInt **bigint, size_t amount);

// Byte shift right.
// Ie INT >> (8*n)
void bigint_byte_shr_memmove(BigInt **bigint_ptr, size_t amount);
