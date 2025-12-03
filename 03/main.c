#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t calc(char *line, int amt) {
  int left = amt;
  uint64_t count = 0;
  int last_use = 0;
  size_t l = strlen(line);
  for (; left > 0;) {
    int max = 0;
    int i = last_use;
    for (; i < l - left; i++) {
      int a = line[i] - '0';
      if (a > max) {
        max = a;
        last_use = i + 1;
      }
    }
    count *= 10;
    count += max;
    left--;
  }
  return count;
}

int main(int argc, char **argv) {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  char *line = NULL;
  for (;;) {
    size_t r;
    ssize_t length;
    if (-1 == (length = getline(&line, &r, stdin))) {
      break;
    }

    part1 += calc(line, 2);
    part2 += calc(line, 12);
  }
  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
  return 0;
}
