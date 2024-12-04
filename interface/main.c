
#include <assert.h>
#include <stdio.h>

#include <immintrin.h>
#include <lib/bigint.h>
#include <lib/output.h>

// A, B, C, D -> B, A, D, C
static inline __m256i _mm256_even_odd_switch_epu32(__m256i a) {
    return _mm256_shuffle_epi32(a, 0b10110001);
}

// A, B, C, D -> B, C, D, A
static inline __m256i _mm256_lrot32_carry_epi32(__m256i a) {
    return _mm256_shuffle_epi32(a, 0b00111001);
}

// A, B, C, D -> D, A, B, C
static inline __m256i _mm256_rrot32_carry_epi32(__m256i a) {
    return _mm256_shuffle_epi32(a, 0b10010011);
}


// Multiply 64bits ints within a vector by the 64bits ints within another vector
// Supports only little endian


void _mul_64x64_to_128(uint64_t a, uint64_t b) {
    printf("%llu %llu\n", a, b);
    __m256i av = _mm256_set1_epi64x(a);
    __m256i bv = _mm256_set1_epi64x(b);

    printf("Vecs: \n");
    print_vec(av);
    print_vec(bv);

    __m256i a_swapped = _mm256_even_odd_switch_epu32(av);
    __m256i b_swapped = _mm256_even_odd_switch_epu32(bv);

    // Offset: 2^64
    __m256i high_mul = _mm256_mul_epu32(av, bv);

    // Offset: 0
    __m256i low_mul = _mm256_mul_epu32(a_swapped, b_swapped);

    printf("\nVert muls: \n");
    print_vec(high_mul);
    print_vec(low_mul);

    // Offset: 2^32
    __m256i cross_1 = _mm256_mul_epu32(bv, a_swapped);
    __m256i cross_2 = _mm256_mul_epu32(av, b_swapped);

    // Combine the cross multiplication
    __m256i cross_add = _mm256_add_epi64(cross_1, cross_2);
    __m256i cross_carry = _mm256_and_si256(_mm256_cmpgt_epi64(cross_1, cross_add), _mm256_set1_epi64x(1));

    printf("Cadd: \n");
    print_vec(cross_1);
    print_vec(cross_2);
    printf("=%llu %llu\n=", *(uint64_t *) &cross_1, *(uint64_t *) &cross_2);
    print_vec(cross_add);
    printf("=%llu\n", *(uint64_t *) &cross_add);
    printf("\n\n\n");

    __m256i cross_sum_and_carry_lo = _mm256_unpacklo_epi64(cross_add, cross_carry);
    print_vec(cross_sum_and_carry_lo);
    __m256i cross_sum_and_carry_hi = _mm256_unpackhi_epi64(cross_add, cross_carry);


    // Construct into epu128 vecs
    // Big endian: low, high
    __m256i out_lo = _mm256_unpacklo_epi64(high_mul, low_mul);
    __m256i out_hi = _mm256_unpackhi_epi64(high_mul, low_mul);

    // Prepare the output vecs to be added properly with the cross multiplication sum
    // As cross multiplication sum is a 64bit value with an offset of 32bits we need
    // to add to the middle of two 32 bit values, the following puts the middle in the
    // bottom vecs temporary
    out_lo = _mm256_lrot32_carry_epi32(out_lo);
    out_hi = _mm256_lrot32_carry_epi32(out_hi);

    out_lo = _mm256_add_epi64(out_lo, cross_sum_and_carry_lo);
    out_hi = _mm256_add_epi64(out_hi, cross_sum_and_carry_hi);

    // Correct for temp shifts
    out_lo = _mm256_rrot32_carry_epi32(out_lo);
    out_hi = _mm256_rrot32_carry_epi32(out_hi);
    printf("Res|: \n");
    print_vec(out_lo);
}

int main() {
    _mul_64x64_to_128((9llu << 32) + 5, 0x11111111);
    // __m256i v = _mm256_setr_epi32(0xA, 0xB, 0xC, 0xD, 0xA, 0xB, 0xC, 0xD);


    // print_vec(_mm256_lrot32_carry_epi32(_mm256_rrot32_carry_epi32(v)));

    // print_vec(_mm256_unpackhi_epi8(v, v));

    return 0;
}
