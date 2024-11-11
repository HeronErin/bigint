#pragma once
#include <endian.h>


#ifndef SEGM_SIZE
// 4096 bytes max segment size
#define SEGMENT_SIZE (1 << 8)
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "debug.h"



struct _Bigint_Segment {
    size_t size;
    char *data;
};

#define BIGINT_SEGMENT_OBJ_SIZE sizeof(struct _Bigint_Segment)

typedef struct {
    // Amount of segments
    size_t size;
    // Amount of possible segments
    size_t capacity;

    // Everything is big endian
    struct _Bigint_Segment segments[];
} Bigint;



static inline Bigint* bigint_new() {
    Bigint* bigint = calloc(1, sizeof(Bigint) + 1024 * BIGINT_SEGMENT_OBJ_SIZE);
    bigint->size = 0;
    bigint->capacity = 4;

    return bigint;
}
static inline Bigint* bigint_with_capacity(size_t capacity) {
    Bigint* bigint = calloc(1, sizeof(Bigint) + capacity * BIGINT_SEGMENT_OBJ_SIZE);
    bigint->size = 0;
    bigint->capacity = capacity;

    return bigint;
}
static inline Bigint* bigint_from(size_t value) {
    Bigint* bi = bigint_with_capacity(1);
    bi->size = 1;
    bi->segments[0].size = sizeof(size_t);
    bi->segments[0].data = calloc(1, SEGMENT_SIZE);
    *(size_t*)(bi->segments[0].data) = htobe64(value);
    return bi;
}
static inline void bigint_grow_for(Bigint** bigint, size_t new_size) {
    Bigint* curr = *bigint;
    if (new_size <= curr->capacity) return;


    // EXPONENTIAL GROWTH!
    size_t grown_cap = (curr->capacity + 1) << 1;
    curr->capacity = grown_cap > new_size ? grown_cap : new_size;
    curr->capacity = curr->capacity > 1024 ? curr->capacity : 1024;
    debug_assert(curr->capacity != 0);

    *bigint = realloc(*bigint, sizeof(Bigint) + curr->capacity * BIGINT_SEGMENT_OBJ_SIZE);
    debug_assert(*bigint != NULL);
}


static inline void bigint_free(Bigint* bigint) {
    for (size_t i = 0; i < bigint->size; i++) free(bigint->segments[i].data);
    free(bigint);
}

char* bigint_hexdump(const Bigint* restrict bigint);

// Discard n segments from the beginning of the bigint.
// Ie: INT >> (SEGMENT_SIZE*8*n)
static inline void bigint_segment_shr(Bigint** bigint, uint16_t amount) {
    Bigint* bi = *bigint;
    if (amount >= bi->size) {
        for (size_t i = 0; i < bi->size; i++) free(bi->segments[i].data);
        bi->size = 0;
        return;
    }

    for (size_t i = 0; i < amount; i++) free(bi->segments[i].data);
    size_t new_size = bi->size - amount;
    bi->size = new_size;
    memmove(bi->segments, bi->segments + amount, new_size * BIGINT_SEGMENT_OBJ_SIZE);
}
// Place n zero filled segments at the beginning of the bigint.
// Ie: INT << (SEGMENT_SIZE*8*n)
static inline void bigint_segment_shl(Bigint** bigint, uint16_t amount) {
    bigint_grow_for(bigint, (*bigint)->size + amount);
    Bigint* bi = *bigint;

    if (bi->size)
        bi->segments[bi->size - 1].size = SEGMENT_SIZE;

}



// Byte shift right. Ie 5 >> (8*n)
void bigint_byte_shr_memcpy(Bigint** bigint_ptr, size_t amount);
// Byte shift left. Ie 5 << (8*n)
char* bigint_byte_shl_memcpy(Bigint** bigint, size_t amount);