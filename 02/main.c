#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool is_invalid(uint64_t number) {
  char str[256];
  snprintf(str, sizeof(str), "%" PRIu64, number);
  int half = strlen(str) / 2;

  if (0 != strlen(str) % 2) {
    return false;
  }

  if (0 == memcmp(str, &str[half], half)) {
    return true;
  }
  return false;
}

bool is_invalid2(uint64_t number) {
  char str[256];
  snprintf(str, sizeof(str), "%" PRIu64, number);

  size_t l = strlen(str);
  for (int i = 1; i < l; i++) {
    char *pattern = str;
    size_t pattern_length = i;

    if (0 != l % pattern_length) {
      continue;
    }

    bool is_same = true;
    for (int j = pattern_length; j < l; j += pattern_length) {
      if (0 != memcmp(pattern, &str[j], pattern_length)) {
        is_same = false;
        break;
      }
    }
    if (is_same) {
      return true;
    }
  }
  return false;
}

int main(int argc, char **argv) {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  for (;;) {
    uint64_t start;
    uint64_t end;
    if (EOF == scanf("%" PRIu64 "-%" PRIu64, &start, &end)) {
      break;
    }
    scanf(",");
    for (uint64_t i = start; i <= end; i++) {
      if (is_invalid(i)) {
        part1 += i;
        part2 += i;
        continue;
      }
      if (is_invalid2(i)) {
        part2 += i;
      }
    }
  }
  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
  return 0;
}
