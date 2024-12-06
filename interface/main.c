
#include <assert.h>
#include <stdio.h>

#include <immintrin.h>
#include <lib/exp_mult.h>
#include <lib/output.h>
#include <lib/debug.h>
#include <stdint.h>


int main() {
    __m256i out[2];
    __m256i a = _mm256_set1_epi64x(69999);
    __m256i b = _mm256_set1_epi64x((1llu << 32) - 1);

    uint64_t start = read_tsc();
    _mm256_mul_epu64(
        a, b,
        out
    );
    uint64_t end = read_tsc();

    // print_vec(out[0]);

    printf("%llu\n", end - start);
    // printf("%llu %llu", ((uint64_t *) out)[0], ((uint64_t *) out)[1]);

    return 0;
}
