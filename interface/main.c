
#include <assert.h>
#include <stdio.h>

#include <immintrin.h>
#include <lib/bigint.h>
#include <lib/output.h>


#include <endian.h>

int main() {
  BigInt *x = bigint_from(1);
  memset(x->segments[0], -1, SEGMENT_SIZE);
  x->size++;
  x->segments[1] = aligned_alloc(32, SEGMENT_SIZE);
  memset(x->segments[1], 0, SEGMENT_SIZE);


  BigInt *y = bigint_from(5);


  bigint_adc(&x, y);

  // bigint_adc(&x, y);
  // bigint_free(bi);
  //
  printf("0x%s\n", bigint_hexdump(x));
  // printf("0x%s\n", bigint_hexdump(y));
  // uint8_t Z = 0;
  // _add_single(&Z, _mm256_set1_epi8(-1), _mm256_set1_epi8(3));


  return 0;
}
