#include "bigint.h"
#include "lib/debug.h"
#include "lib/output.h"
#include <stdint.h>
#include <stdio.h>

char* bigint_hexdump(const Bigint* restrict bigint) {
    // Upperbound of possible output sizes
    char* buf = malloc(bigint->size * SEGMENT_SIZE * 2 + 1);
    char* curr_dest = buf;
    for (size_t segment_index = bigint->size-1; segment_index != -1; segment_index--) {
        const struct _Bigint_Segment seg = bigint->segments[segment_index];

        dump_hex_into(curr_dest, seg.data, seg.size);
        curr_dest += seg.size*2;
    }
    *curr_dest = 0;

    return buf;
}

void bigint_byte_shr_memcpy(Bigint** bigint_ptr, size_t amount) {
    if (!amount) return;


    if (amount > SEGMENT_SIZE) {
        bigint_segment_shr(bigint_ptr, amount / SEGMENT_SIZE);
        amount %= SEGMENT_SIZE;
    }

    Bigint* bigint = *bigint_ptr;

    // Buffer where we temporarily put bytes to be put for the next segment
    char* temp;
    if (bigint->size <= 1) temp = NULL;
    else if (amount > 1024) temp = malloc(amount);
    else temp = alloca(amount);

    size_t amount_in_temp = 0;
    size_t current_amount_to_temp = 0;

    for (size_t segment_index = bigint->size-1; segment_index != -1; segment_index--) {
        const struct _Bigint_Segment seg = bigint->segments[segment_index];

        if (segment_index != 0) {
            current_amount_to_temp = seg.size > amount ? amount : seg.size;
            memcpy(temp, seg.data + seg.size - current_amount_to_temp, current_amount_to_temp);
        }
        if (seg.size > amount) {
            memmove(seg.data + amount, seg.data, seg.size - amount);
            memcpy(seg.data, temp, amount_in_temp);
        }else {
            debug_assert(segment_index == bigint->size-1);
            free(seg.data);
            bigint->size--;
            bigint->segments[segment_index].data = NULL;
        }
        amount_in_temp = current_amount_to_temp;
    }

    if (amount > 1024) free(temp);
}
