#pragma once
#include <stdint.h>

// Technically unnecessary, however it doesn't hurt anything
#ifdef DEBUG
#include <assert.h>
#define debug_assert(EXPR) assert(EXPR)
#else
#define debug_assert(EXPR) {}
#endif


static inline uint64_t read_tsc() {
    unsigned int lo, hi;
    // Use rdtscp to serialize the instruction for more accurate measurement
    asm volatile ("rdtscp" : "=a"(lo), "=d"(hi) :: "rcx");
    asm volatile ("lfence"); // Prevent out-of-order execution
    return ((uint64_t) hi << 32) | lo;
}
