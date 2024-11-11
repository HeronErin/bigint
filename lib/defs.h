#pragma once

#ifndef SEGM_SIZE
// 5kb max segment size
#define SEGMENT_SIZE 1024*5
#endif

#include <stdlib.h>
#include "debug.h"





struct _bigint_segment{
    // Size of upcoming data NOT TOTAL SIZE
    size_t size;
    unsigned char data[];
};

typedef struct {
    size_t size;
    size_t capacity;
    struct _bigint_segment* segments[];
} Bigint;

static inline struct _bigint_segment* _bigint_segment_with_size_rnd(size_t size) {
    struct _bigint_segment* ret = malloc(size + sizeof(struct _bigint_segment));
    debug_assert(size <= SEGMENT_SIZE);
    ret->size = size;
    return ret;
}
static inline struct _bigint_segment* _bigint_segment_with_size_zero(size_t size) {
    struct _bigint_segment* ret = calloc(1, size + sizeof(struct _bigint_segment));
    debug_assert(size <= SEGMENT_SIZE);
    ret->size = size;
    return ret;
}

// Returns a bigint with the ability to store N segments
// Caller takes ownership of bigint
static inline Bigint* bigint_with_capacity(size_t capacity) {
    Bigint* ret = malloc(sizeof(Bigint) + sizeof(struct _bigint_segment*) * capacity);
    ret->size = 0;
    ret->capacity = capacity;
    return ret;
}

static inline Bigint* bigint_from(size_t uint) {
    Bigint* ret = bigint_with_capacity(1);
    ret->segments[0] = _bigint_segment_with_size_zero(SEGMENT_SIZE);
    ret->segments[0]->size = sizeof(size_t);
    ret->segments[0]->data[SEGMENT_SIZE-sizeof(size_t)] = uint;
    ret->size = 0;

    return ret;
}

