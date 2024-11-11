
#include <stdio.h>

#include <immintrin.h>
#include <lib/output.h>

#include "benchmark.h"
#include <unistd.h>


void test() {

  usleep(0.2e6);
}


int main() {
  // printf("t\n");
  printf("Estimated time: %f\n", benchmark(&test, 10, 5.0));

  return 0;
}