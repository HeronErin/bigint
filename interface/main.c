
#include <stdio.h>

#include <immintrin.h>
#include <lib/bigint.h>



int main() {

  Bigint* bi = bigint_from(0x123456789abcdef);

  // ((size_t*) bi->segments[0].data)[0] = 0x12345678;

  // ((size_t*) bi->segments[0].data)[3] = -1;
  //
  //
  bigint_segment_shl(&bi, 1);
  // bigint_segment_shr(&bi, 1);
  //
  // printf("#1: %x\n", (unsigned char)bi->segments[0].data[0]);
  // printf("%lu\n%s\n", bi->size, bigint_hexdump(bi));

  bigint_f_prune(bi);
  printf("%lu\n%s\n", bi->size, bigint_hexdump(bi));
  bigint_byte_shr_memmove(&bi, 32);
  bigint_f_prune(bi);
  printf("%lu\n%s\n", bi->size, bigint_hexdump(bi));


  // bigint_free(bi);

  // int i =  __builtin_clz(0);
  // printf("%i\n", i);

  return 0;
}