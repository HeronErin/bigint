
#include <stdio.h>

#include <immintrin.h>
#include <lib/bigint.h>



int main() {

  Bigint* bi = bigint_from(0x1690000);
  bigint_segment_shl(&bi, 1);
  printf("%lu\n%s\n", bi->size, bigint_hexdump(bi));

  return 0;
}