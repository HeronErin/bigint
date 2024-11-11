#pragma once

#include "defs.h"

void bin_to_hex_32(void* restrict dst, const void* restrict src);
char* dump_hex(const void* restrict src, size_t n);