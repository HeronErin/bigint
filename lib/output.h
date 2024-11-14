#pragma once
#include <immintrin.h>


static inline __m256i _reverse(__m256i dt) {
    const __m256i shuffle = _mm256_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, // first 128-bit lane
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 // second 128-bit lane
    );
    __m256i shuffed = _mm256_shuffle_epi8(dt, shuffle);

    return _mm256_permute2x128_si256(shuffed, shuffed, 1);
}

static inline __m256i _nibbles_to_hex(__m256i nibbles) {
    // Greater than 9 gives an invalid result, the rest is to fix that
    __m256i pseudo_hex = _mm256_add_epi8(nibbles, _mm256_set1_epi8('0'));

    // 0xFF if greater than 9
    __m256i msk = _mm256_cmpgt_epi8(nibbles, _mm256_set1_epi8(9));

    // If greater than 9, set the mask to be 'A' - '0' - 10
    __m256i letter_addr = _mm256_and_si256(msk, _mm256_set1_epi8('A' - '0' - 10));
    return _mm256_add_epi8(pseudo_hex, letter_addr);
}

static inline void _bin_to_hex_32(void *restrict dst, __m256i data) {
    // Get lower halfs of the binary isolated and convert to ascii
    __m256i lower_hex = _nibbles_to_hex(_mm256_and_si256(data, _mm256_set1_epi8(0x0F)));

    // Gey upper halfs of binary isolated, shift to lower bytes, and convert to ascii
    __m256i upper_hex = _nibbles_to_hex(_mm256_srli_epi16(_mm256_and_si256(data, _mm256_set1_epi8(0xF0)), 4));

    // Interleave bytes within 128-bit lanes (lower and upper halves handled separately)
    __m256i txt_lo = _mm256_unpacklo_epi8(upper_hex, lower_hex);
    __m256i txt_hi = _mm256_unpackhi_epi8(upper_hex, lower_hex);


    __m256i result_lo = _mm256_permute2x128_si256(txt_lo, txt_hi, 0x20);
    __m256i result_hi = _mm256_permute2x128_si256(txt_lo, txt_hi, 0x31);


    _mm256_storeu_si256((__m256i *) dst, result_lo);
    _mm256_storeu_si256((__m256i *) dst + 1, result_hi);;
}

// Safety: dst MUST BE >= n*2 + 2
void dump_hex_into(void *restrict dst, const void *restrict src, size_t n);

char *dump_hex(const void *restrict src, size_t n);
