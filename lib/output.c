#include <string.h>
#include "output.h"


void dump_hex_into(void *restrict dst, const void *restrict src, size_t n) {
    char *cdest = dst;
    while (n >= 32) {
        bin_to_hex_32(cdest, _mm256_loadu_si256((__m256i *) src));
        cdest += 64;
        src += 32;
        n -= 32;
    }
    if (n) {
        char temp[64];
        // Just ignore the fact we are reading out of bounds
        bin_to_hex_32(temp, _mm256_loadu_si256((__m256i *) src));
        memcpy(cdest, temp, n * 2);
        cdest += n * 2;
    }
    cdest[0] = 0;
}


char *dump_hex(const void *restrict src, size_t n) {
    char *dst = malloc(n * 2 + 1);
    dump_hex_into(dst, src, n);
    return dst;
}


