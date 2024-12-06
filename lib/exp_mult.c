#include "exp_mult.h"
#include <immintrin.h>


// A, B, C, D -> B, A, D, C
static inline __m256i _mm256_even_odd_switch_epu32(__m256i a) {
    return _mm256_shuffle_epi32(a, 0b10110001);
}

// A, B, C, D -> B, C, D, A
static inline __m256i _mm256_lrot32_carry_epu32(__m256i a) {
    return _mm256_shuffle_epi32(a, 0b00111001);
}

// A, B, C, D -> D, A, B, C
static inline __m256i _mm256_rrot32_carry_epu32(__m256i a) {
    return _mm256_shuffle_epi32(a, 0b10010011);
}


// Multiply 64bits ints within a vector by the 64bits ints within another vector
// Supports only little endian
void _mm256_mul_epu64(__m256i a, __m256i b, __m256i *out) {
    // Swap even and odd 32-bit values in the input vectors
    __m256i a_swapped = _mm256_even_odd_switch_epu32(a);
    __m256i b_swapped = _mm256_even_odd_switch_epu32(b);

    // Perform 64-bit multiplication for the high and low parts
    __m256i high_mul = _mm256_mul_epu32(a, b); // Offset: 2^64
    __m256i low_mul = _mm256_mul_epu32(a_swapped, b_swapped); // Offset: 0

    // Perform cross multiplication to handle middle 64 bits
    __m256i cross_1 = _mm256_mul_epu32(b, a_swapped); // Cross multiplication part 1
    __m256i cross_2 = _mm256_mul_epu32(a, b_swapped); // Cross multiplication part 2

    // Add the results of cross multiplication
    __m256i cross_add = _mm256_add_epi64(cross_1, cross_2);

    // Detect if the addition has overflowed, if so set to one
    __m256i cross_carry = _mm256_and_si256(
        _mm256_cmpgt_epi64(cross_1, cross_add),
        _mm256_set1_epi64x(1)
    );

    // Separate cross addition and carry into low and high parts
    __m256i cross_sum_and_carry_lo = _mm256_unpacklo_epi64(cross_add, cross_carry);
    __m256i cross_sum_and_carry_hi = _mm256_unpackhi_epi64(cross_add, cross_carry);

    // Construct intermediate 128-bit results from high and low multiplications
    __m256i out_lo = _mm256_unpacklo_epi64(high_mul, low_mul);
    __m256i out_hi = _mm256_unpackhi_epi64(high_mul, low_mul);

    // Adjust the results to align with the cross multiplication sum
    out_lo = _mm256_lrot32_carry_epu32(out_lo);
    out_hi = _mm256_lrot32_carry_epu32(out_hi);

    // Add the cross multiplication sum to the intermediate results
    // Carry handing is not actually needed here surprisingly!
    // To see why, look at the binary when multiplying 0xFF by itself!
    out_lo = _mm256_add_epi64(out_lo, cross_sum_and_carry_lo);
    out_hi = _mm256_add_epi64(out_hi, cross_sum_and_carry_hi);

    // Reverse the temporary alignment adjustments
    out_lo = _mm256_rrot32_carry_epu32(out_lo);
    out_hi = _mm256_rrot32_carry_epu32(out_hi);

    out[0] = out_lo;
    out[1] = out_hi;
}
