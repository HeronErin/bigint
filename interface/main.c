
#include <assert.h>
#include <stdio.h>

#include <immintrin.h>
#include <lib/bigint.h>
#include <lib/output.h>


static inline __m256i _odd_even_switch(__m256i a) {
    return _mm256_shuffle_epi32(a, 0b10110001);
}

// Multiply 64bits ints within a vector by the 64bits ints within another vector
// Supports only little endian
void _mul_64x64_to_128(__m256i a, __m256i b, __m256i *out) {
    const __m256i aswap = _odd_even_switch(a);
    const __m256i bswap = _odd_even_switch(b);

    const __m256i low_mul = _mm256_mul_epu32(a, b); // Offset: 0
    const __m256i hi_mul = _mm256_mul_epu32(aswap, bswap); // Offset: 2^64

    const __m256i diag_mul1 = _mm256_mul_epu32(aswap, b); // Offset: 2^32
    const __m256i diag_mul2 = _mm256_mul_epu32(a, bswap); // Offset: 2^32

    const __m256i *low_mul_ptr = &low_mul;
    const __m256i *hi_mul_ptr = &hi_mul;
    const __m256i *diag_mul1_ptr = &diag_mul1;
    const __m256i *diag_mul2_ptr = &diag_mul2;


    uint64_t *out64 = (uint64_t *) out;

    // At this point all we need to do is add 'em all up. Unfortunately avx2 fails us here
#pragma unroll
    for (int i = 0; i < 4; i++) {
        out64[1] = ((uint64_t *) low_mul_ptr)[i];
        out64[0] = ((uint64_t *) hi_mul_ptr)[i];
        uint64_t *midout = (uint64_t *) ((uint32_t *) out64 + 3);
        char carry = _addcarryx_u64(0, midout[0], ((uint64_t *) diag_mul1_ptr)[i], (unsigned long long *) midout);
        carry += _addcarryx_u64(0, midout[0], ((uint64_t *) diag_mul2_ptr)[i], (unsigned long long *) midout);
        *((unsigned char *) out64) += carry;

        out64 += 2;
    }
}

int main() {
    __m256i out[2];
    // __m256i a = _mm256_setr_epi32(2, 2, 0, 0, 0, 0, 0, 0);
    __m256i b = _mm256_setr_epi64x(99999999999999, 0, 0, 0);
    // print_vec(a);
    // print_vec(b);

    _mul_64x64_to_128(b, b, out);

    // print_vec(out[0]);
    // print_vec(out[1]);
    printf("%llu %%2^64 \t\t %llu //2^64\n", ((uint64_t *) &out[0])[0], ((uint64_t *) &out[0])[1]);


    return 0;
}
