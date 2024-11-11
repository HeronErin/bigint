
#include <stdio.h>

#include <immintrin.h>
#include <lib/output.h>

static inline __m256i _nibbles_to_hex(__m256i nibbles) {
  // Greater than 9 gives an invalid result, the rest is to fix that
  __m256i pseudo_hex = _mm256_add_epi8(nibbles, _mm256_set1_epi8('0'));

  // 0xFF if greater than 9
  __m256i msk = _mm256_cmpgt_epi8(nibbles, _mm256_set1_epi8(9));

  // If greater than 9, set the mask to be 'A' - '0' - 10
  __m256i letter_addr = _mm256_and_si256(msk, _mm256_set1_epi8('A' - '0' - 10));
  return _mm256_add_epi8(pseudo_hex, letter_addr);
}
int main() {
  char* x = alloca(32);
  for (char i = 0; i < 32; i++) x[i]=(i);// | 0xF0;
  char out[65] = {0};
  bin_to_hex_32(out, x);

  // printf("\n%s\n", out);
  for (int i = 0; i < 32; i++) printf("%c%c ", ((unsigned char*) &out)[i*2], ((unsigned char*) &out)[i*2+1]);


  return 0;
}