#include "bigint.h"
#include "lib/output.h"
#include <stdint.h>
#include <stdio.h>
#include <immintrin.h>

char *bigint_hexdump(const BigInt *restrict bigint) {
    // Upperbound of possible output sizes
    char *buf = malloc(bigint->size * SEGMENT_SIZE * 2 + 1);
    char *curr_dest = buf;
    char do_print = 0;


    for (size_t segment_index = bigint->size - 1; segment_index != -1; segment_index--) {
        char *seg = bigint->segments[segment_index];
        for (size_t offset = SEGMENT_SIZE - 32; offset != -32; offset -= 32) {
            __m256i data = _mm256_load_si256((__m256i *) (seg + offset));
            do_print |= _mm256_movemask_epi8(_mm256_cmpeq_epi8(data, _mm256_setzero_si256())) != -1;

            if (!do_print) continue;

            bin_to_hex_32(curr_dest, _reverse(data));
            curr_dest += 64;
        }
    }
    *curr_dest = 0;

    return buf;
}

void bigint_segment_shl(BigInt **bigint, uint16_t amount) {
    if (amount == 0)
        return;

    bigint_grow_for(bigint, (*bigint)->size + amount);
    BigInt *bi = *bigint;

    if (!bi->size)
        return;

    bi->size += (size_t) amount;

    memmove(bi->segments + amount, bi->segments, sizeof(char *) * amount);

    for (size_t i = 0; i < amount; i++) {
        char *segment = aligned_alloc(32, SEGMENT_SIZE);
        memset(segment, 0, SEGMENT_SIZE);

        bi->segments[i] = segment;
    }
}

void bigint_segment_shr(BigInt **bigint, uint16_t amount) {
    if (amount == 0)
        return;

    BigInt *bi = *bigint;
    if (amount >= bi->size) {
        for (size_t i = 0; i < bi->size; i++)
            free(bi->segments[i]);

        bi->size = 0;
        return;
    }

    for (size_t i = 0; i < amount; i++)
        free(bi->segments[i]);

    size_t new_size = bi->size - amount;
    bi->size = new_size;
    memmove(bi->segments, bi->segments + amount, new_size * sizeof(char *));
}

void bigint_byte_shr_memmove(BigInt **bigint_ptr, size_t amount) {
    BigInt *bi = *bigint_ptr;
    if (amount >= SEGMENT_SIZE) {
        bigint_segment_shr(bigint_ptr, amount / SEGMENT_SIZE);
        amount = amount % SEGMENT_SIZE;
    }

    if (amount == 0)
        return;

    for (size_t i = 0; i < bi->size; i++) {
        char *segment = bi->segments[i];
        memmove(segment, segment + amount, SEGMENT_SIZE - amount);
        if (i + 1 == bi->size)
            memset(segment + SEGMENT_SIZE - amount, 0, amount);
        else
            memcpy(segment + SEGMENT_SIZE - amount, bi->segments[i + 1], amount);
    }
}

void bigint_byte_shr_bsrli(BigInt **ptr, size_t cnt) {
    const __m256i mask = _mm256_set_epi8(
        0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    );

    BigInt *bi = *ptr;
    __m256i lo = _mm256_setzero_si256();
    for (int i = 0; i < cnt; i++) {
        for (int j = 0; j < bi->size; j++) {
            char *segment = bi->segments[j];
            for (int k = 0; k < SEGMENT_SIZE / 32; k++) {
                __m256i vec = ((__m256i *) segment)[k];
                __m256i carry = _mm256_and_si256(vec, mask);
                vec = _mm256_bsrli_epi128(vec, 1);
                carry = _mm256_permute4x64_epi64(carry, 0b01101100);
                carry = _mm256_bslli_epi128(carry, 15);
                __m256i hi = _mm256_insert_epi8(carry, 0, 0);
                vec = _mm256_or_si256(vec, lo);
                lo = _mm256_insert_epi8(carry, 0, 16);
                vec = _mm256_or_si256(vec, hi);
                ((__m256i *) segment)[k] = vec;
            }
        }
    }
}

char _segment_adc(unsigned char carry, void *x, void *y) {
    uint64_t *dst = x;
    uint64_t *src = y;
    uint64_t *end = (uint64_t *) ((char *) x + SEGMENT_SIZE);


    while (dst != end) {
        carry = _addcarryx_u64(carry, dst[0], src[0], (unsigned long long *) &dst[0]);
        carry = _addcarryx_u64(carry, dst[1], src[1], (unsigned long long *) &dst[1]);
        carry = _addcarryx_u64(carry, dst[2], src[2], (unsigned long long *) &dst[2]);
        carry = _addcarryx_u64(carry, dst[3], src[3], (unsigned long long *) &dst[3]);
        dst += 4;
        src += 4;
    }
    return carry;
}

static inline char _expanded_adc_carry(unsigned char carry, void *x) {
    uint64_t *dst = x;
    uint64_t *end = (uint64_t *) ((char *) x + SEGMENT_SIZE);


    while (dst != end && carry) {
        carry = _addcarryx_u64(carry, dst[0], 0, (unsigned long long *) &dst[0]);
        carry = _addcarryx_u64(carry, dst[1], 0, (unsigned long long *) &dst[1]);
        carry = _addcarryx_u64(carry, dst[2], 0, (unsigned long long *) &dst[2]);
        carry = _addcarryx_u64(carry, dst[3], 0, (unsigned long long *) &dst[3]);
        dst += 4;
    }
    return carry;
}

static inline char _segment_sbc(unsigned char carry, void *x, void *y) {
    uint64_t *dst = x;
    uint64_t *src = y;
    uint64_t *end = (uint64_t *) ((char *) x + SEGMENT_SIZE);


    while (dst != end) {
        carry = _subborrow_u64(carry, dst[0], src[0], (unsigned long long *) &dst[0]);
        carry = _subborrow_u64(carry, dst[1], src[1], (unsigned long long *) &dst[1]);
        carry = _subborrow_u64(carry, dst[2], src[2], (unsigned long long *) &dst[2]);
        carry = _subborrow_u64(carry, dst[3], src[3], (unsigned long long *) &dst[3]);
        dst += 4;
        src += 4;
    }
    return carry;
}

static inline char _expanded_sbc_carry(unsigned char carry, void *x) {
    uint64_t *dst = x;
    uint64_t *end = (uint64_t *) ((char *) x + SEGMENT_SIZE);


    while (dst != end) {
        carry = _subborrow_u64(carry, dst[0], 0, (unsigned long long *) &dst[0]);
        carry = _subborrow_u64(carry, dst[1], 0, (unsigned long long *) &dst[1]);
        carry = _subborrow_u64(carry, dst[2], 0, (unsigned long long *) &dst[2]);
        carry = _subborrow_u64(carry, dst[3], 0, (unsigned long long *) &dst[3]);
        dst += 4;
    }
    return carry;
}

static inline __m256i _swap_lanes(__m256i a) {
    return _mm256_permute4x64_epi64(a, 0b01001110);
}


// *a +=  b
void bigint_adc(BigInt **a, BigInt *b) {
    BigInt *a_ = *a;
    size_t min_size = a_->size < b->size ? a_->size : b->size;
    unsigned char carry = 0;
    for (size_t i = 0; i < min_size; i++)
        carry = _segment_adc(carry, a_->segments[i], b->segments[i]);


    if (a_->size > b->size) {
        // Carry the Bitch onward
        for (size_t i = min_size; i < a_->size && carry; i++)
            carry = _expanded_adc_carry(carry, a_->segments[i]);
    } else if (a_->size < b->size) {
        // a's new segments will just be from b + carry

        bigint_grow_for(a, b->capacity);
        a_ = *a;
        a_->size = b->size;
        for (size_t i = min_size; i < b->size; i++) {
            char *new_seg = aligned_alloc(32, SEGMENT_SIZE);
            memcpy(new_seg, b->segments[i], SEGMENT_SIZE);
            a_->segments[i] = new_seg;
        }

        for (size_t i = min_size; i < b->size; i++)
            carry = _expanded_adc_carry(carry, a_->segments[i]);
    }
    if (!carry) return;

    bigint_grow_for(a, a_->size + 1);
    a_ = *a;
    char *new_seg = aligned_alloc(32, SEGMENT_SIZE);
    memset(new_seg, 0, SEGMENT_SIZE);
    *new_seg = carry;
    a_->segments[a_->size] = new_seg;
    a_->size++;
}

char bigint_sbc(BigInt **a, BigInt *b) {
    BigInt *a_ = *a;

    // Pad A to be as large as B
    if (a_->size < b->size) {
        for (size_t i = a_->size; i < b->size; i++) {
            char *new_seg = aligned_alloc(32, SEGMENT_SIZE);
            memset(new_seg, 0, SEGMENT_SIZE);
            a_->segments[i] = new_seg;
        }
        a_->size = b->size;
    }
    unsigned char carry = 0;
    for (size_t i = 0; i < b->size; i++)
        carry = _segment_sbc(carry, a_->segments[i], b->segments[i]);


    if (a_->size > b->size)
        for (size_t i = b->size; i < a_->size; i++)
            _expanded_sbc_carry(carry, a_->segments[i]);
    return carry;
}
