#pragma once
#include <stddef.h>

void bin_to_hex_32(void* restrict dst, const void* restrict src);
void bin_to_hex_32_r(void* restrict dst, const void* restrict src);

// Safety: dst MUST BE >= n*2 + 2
void dump_hex_into(void* restrict dst, const void* restrict src, size_t n);
char* dump_hex(const void* restrict src, size_t n);
