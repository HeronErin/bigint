#include "lib/benchmark.h"
#include "lib/bigint.h"
#include <stdio.h>
#include <unistd.h>


void memmove_benchmark() {
  static BigInt* bi = NULL;
  if (bi == NULL) bi = bigint_zeroed_of_size(100000);
//
  bigint_byte_shr_memmove(&bi, 3);
//  bigint_byte_shr_memmove(&bi, 3);
//  bigint_byte_shr_memmove(&bi, 3);
//  bigint_byte_shr_memmove(&bi, 3);
//  bigint_byte_shr_memmove(&bi, 3);
//  bigint_byte_shr_memmove(&bi, 3);
//  bigint_byte_shr_memmove(&bi, 3);
}



int main() {
  printf("Average : %zu", ubenchmark(&memmove_benchmark, 100, 3));
}
