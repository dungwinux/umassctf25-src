extern "C" int special(unsigned char *__restrict__ dst,
                       const unsigned char *__restrict__ src, unsigned length);

#include <stdio.h>
#include <string.h>

void test_collision() {
  unsigned char buf[0x100];
  unsigned char input[0x9f];
  int count[0x100];
  memset(buf, '\0', 0x100);
  memset(input, '\0', 0x9f);
  for (int i = 0; i < 0x9f; ++i) {
    printf("Checking position %d\n", i);
    memset(count, '\0', 0x100 * sizeof(int));
    for (int j = 0; j < 0x100; ++j) {
      input[i] = j;
      special(buf, input, 0x9f);
      ++count[buf[i]];
      // printf("%02hhx ", buf[i]);
      // if (0 == ((j + 1) & 0x1f))
      //   putchar('\n');
    }
    for (int k = 0; k < 0x100; ++k) {
      if ((count[k] != 0) && (count[k] != 1)) {
        printf("duplicated entry: %02x (%d)\n", k, count[k]);
      }
      // printf("%02d\n", count[k]);
    }
  }
}

int main() { test_collision(); }
