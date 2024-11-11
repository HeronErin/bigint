#include "math.h"
#include <immintrin.h>
#include <string.h>

#include <stdio.h>
static inline  __attribute((always_inline)) __m256i _nibbles_to_hex(__m256i nibbles) {
    // Greater than 9 gives an invalid result, the rest is to fix that
    __m256i pseudo_hex = _mm256_add_epi8(nibbles, _mm256_set1_epi8('0'));

    // 0xFF if greater than 9
    __m256i msk = _mm256_cmpgt_epi8(nibbles, _mm256_set1_epi8(9));

    // If greater than 9, set the mask to be 'A' - '0' - 10
    __m256i letter_addr = _mm256_and_si256(msk, _mm256_set1_epi8('A' - '0' - 10));
    return _mm256_add_epi8(pseudo_hex, letter_addr);
}
  static inline __attribute((always_inline)) void _bin_to_hex_32(void* restrict dst, const void* restrict src){
    __m256i data = _mm256_loadu_si256((__m256i *) src);

    // Get lower halfs of the binary isolated and convert to ascii
    __m256i lower_hex = _nibbles_to_hex( _mm256_and_si256(data, _mm256_set1_epi8(0x0F)) );

    // Gey upper halfs of binary isolated, shift to lower bytes, and convert to ascii
    __m256i upper_hex = _nibbles_to_hex( _mm256_srli_epi16(_mm256_and_si256(data, _mm256_set1_epi8(0xF0)), 4) );

    // Interleave bytes within 128-bit lanes (lower and upper halves handled separately)
    __m256i txt_lo = _mm256_unpacklo_epi8(upper_hex, lower_hex);
    __m256i txt_hi = _mm256_unpackhi_epi8(upper_hex, lower_hex);

    // Permute 128-bit lanes to interleave across the full 256-bit width
    __m256i result_lo = _mm256_permute2x128_si256(txt_lo, txt_hi, 0x20);
    __m256i result_hi = _mm256_permute2x128_si256(txt_lo, txt_hi, 0x31);

    _mm256_storeu_si256((__m256i*)dst, result_lo);
    _mm256_storeu_si256((__m256i*)dst + 1, result_hi);;
}

void bin_to_hex_32(void* restrict dst, const void* restrict src) { _bin_to_hex_32(dst, src); }
char* dump_hex(const void* restrict src, size_t n) {
    char* dst = calloc(1, n * 2 + 1);

    char* cdest = dst;
    while (n >= 32) {
        _bin_to_hex_32(cdest, src);
        cdest += 64;
        src += 32;
        n-=32;
    }
    if (n) {
        char temp[64];
        // Just ignore the fact we are reading out of bounds
        _bin_to_hex_32(temp, src);
        memcpy(cdest, temp, n*2);
        cdest += n*2;
    }
    cdest[0] = 0;
    return dst;
}
