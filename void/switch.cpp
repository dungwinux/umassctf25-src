#define NAKED __attribute__((naked))

extern "C" int special(unsigned char *__restrict__ dst,
                       const unsigned char *__restrict__ src, unsigned length);

#include <stdio.h>
#include <string.h>

template <unsigned int N, typename T = int, T Start = 0> struct A {
  T arr[N];
  unsigned int length;
  constexpr A() : arr{}, length{N} {
    for (auto i = 0; i != N; ++i) {
      arr[i] = Start + i;
    }
  }
};

// constexpr const char s[] =
//     "\x31\x3c\x47\x52\x5d\x68\x73\x76\x31\x3c\x47\x52\x5d\x68\x73\x76\x31\x3c"
//     "\x47\x52\x5d\x68\x73\x76\x31\x3c\x47\x52\x5d\x68\x73\x76\x31\x3c\x47\x52"
//     "\x5d\x68\x73\x76";
// constexpr auto size = sizeof(s) - 1;
constexpr const auto sa = A<256, unsigned char, 0x0>();
int main() {
  unsigned char x[0x100];
  int result = special(x, sa.arr, sa.length);
  if (result < 0) {
    printf("Processor is not supported!");
    return -1;
  }
  for (int i = 0; i < sa.length; ++i) {
    printf("\t[%03d]=%02x", sa.arr[i], x[i]);
  }
  putchar('\n');
  // for (int i = 0; i < 256; ++i) {
  //   printf("%02x", i);
  //   for (int j = 0; j < 256; ++j) {
  //     if (x[j] == i) {
  //       printf("=[%02d]", j);
  //     }
  //   }
  //   putchar('\n');
  // }

  // Should be in the range of [0x30..0x7b)
  const unsigned char flag[] =
      "0DD1y_H4nD_0ptiMi2eD_X8664_pr0gr4M_by_m3_;>_Soy4jGPHr3g_________________"
      "______________________________";

  for (int i = 0; i < sizeof(flag) - 1; ++i) {
    if ((flag[i] < 0x30) || (flag[i] >= 0x7b)) {
      printf("warning: invalid char at %d [%c=0x%x]\n", i, flag[i], flag[i]);
    }
  }

  const unsigned int size = sizeof(flag) - 1;
  memset(x, 0, 0x100);
  result = special(x, flag, size);
  putchar('"');
  for (int i = 0; i < size; ++i) {
    printf("\\x%02x", x[i]);
  }
  putchar('"');
}
