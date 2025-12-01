#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  int64_t dial = 50;
  for (;;) {
    char d;
    int64_t number;
    if (EOF == fscanf(stdin, "%c%" PRIi64 "\n", &d, &number)) {
      break;
    }
    bool positive = ('R' == d);

    int64_t prev = dial;

    part2 += number / 100;
    if (0 == prev && 0 == number % 100) {
      part2--;
    }
    number %= 100;
    dial += (positive) ? (number) : (-1 * number);

    if ((dial < 0 || dial > 100) && 0 != prev) {
      part2++;
    }
    if (0 > dial) {
      dial = 100 + dial;
    }
    dial %= 100;
    if (0 == dial) {
      part1++;
    }
  }
  part2 += part1;
  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
  return 0;
}
