#include "bigint.h"
#include "lib/debug.h"
#include "lib/output.h"
#include <stdint.h>
#include <stdio.h>

char* bigint_hexdump(const BigInt* restrict bigint) {
    // Upperbound of possible output sizes
    char* buf = malloc(bigint->size * SEGMENT_SIZE * 2 + 1);
    char* curr_dest = buf;
    for (size_t segment_index = bigint->size-1; segment_index != -1; segment_index--) {
        const struct _BigInt_Segment seg = bigint->segments[segment_index];

        debug_assert(seg.size % 32 == 0);
        for (size_t offset = seg.size - 32; offset != -32; offset -= 32) {
            bin_to_hex_32_r(curr_dest, seg.data + offset);
            curr_dest += 64;
        }

    }
    *curr_dest = 0;

    return buf;
}

void bigint_segment_shl(BigInt** bigint, uint16_t amount) {
    if (amount == 0) 
        return;
        
    bigint_grow_for(bigint, (*bigint)->size + amount);
    BigInt* bi = *bigint;

    if (!bi->size) 
        return;

    bi->size += (size_t)amount;

    memmove(bi->segments + amount, bi->segments, BIGINT_SEGMENT_OBJ_SIZE * amount);

    for (size_t i = 0; i < amount; i++) {
        char* segment = aligned_alloc(32, SEGMENT_SIZE);
        memset(segment, 0,  SEGMENT_SIZE);

        bi->segments[i].size = SEGMENT_SIZE;
        bi->segments[i].data = segment;
    }
}

void bigint_segment_shr(BigInt** bigint, uint16_t amount) {
    if (amount == 0) 
        return;

    BigInt* bi = *bigint;
    if (amount >= bi->size) {
        for (size_t i = 0; i < bi->size; i++) 
            free(bi->segments[i].data);

        bi->size = 0;
        return;
    }

    for (size_t i = 0; i < amount; i++) 
        free(bi->segments[i].data);

    size_t new_size = bi->size - amount;
    bi->size = new_size;
    memmove(bi->segments, bi->segments + amount, new_size * BIGINT_SEGMENT_OBJ_SIZE);
}

void bigint_f_prune(BigInt* bigint) {
    start:
    if (bigint->size == 0) 
        return;

    struct _BigInt_Segment* segment = &bigint->segments[bigint->size - 1];
    const char* end = segment->data;
    size_t size = segment->size;
    char* curr = segment->data + size - sizeof(size_t);

    for (;curr >= end; curr-=sizeof(size_t)) {
        size_t item = *(size_t*)curr;
        if (item) break;
        size -= sizeof(size_t);
    }

    if (end  == curr + sizeof(size_t)) {
        free(segment->data);
        bigint->size--;
        goto start;
    }

    size_t r = size % 32;
    segment->size = size + (r ? 32 - r : 0);
}

void bigint_byte_shr_memmove(BigInt** bigint_ptr, size_t amount) {
    BigInt* bi = *bigint_ptr;
    if (amount >= SEGMENT_SIZE) {
        bigint_segment_shr(bigint_ptr, amount / SEGMENT_SIZE);
        amount = amount % SEGMENT_SIZE;
    }

    if (amount == 0) 
        return;

    for (size_t i = 0; i < bi->size; i++) {
        struct _BigInt_Segment* segment = &bi->segments[i];
        memmove(segment->data, segment->data + amount, segment->size - amount);
        if (i+1 == bi->size) 
            memset(segment->data + segment->size - amount, 0, amount);
        else {
            memcpy(segment->data + segment->size - amount, bi->segments[i+1].data, amount);
            if (amount > bi->segments[i+1].size)
                memset(segment->data + segment->size - amount + bi->segments[i+1].size, 0, amount - bi->segments[i+1].size);
        }
    }
}

void bigint_byte_shr_bsrli(BigInt** ptr, size_t cnt)
{
    
}