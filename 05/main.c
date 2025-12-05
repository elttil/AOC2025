#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct range {
  uint64_t start;
  uint64_t end;
};

struct range *list = NULL;
size_t list_size = 0;
size_t list_one = 0;

bool isspoiled_calc(uint64_t a) {
  for (size_t i = 0; i < list_one; i++) {
    if (a >= list[i].start && a <= list[i].end) {
      return false;
    }
  }
  return true;
}

void add_list(uint64_t s, uint64_t e) {
  if (list_one >= list_size) {
    list_size += 4096;
    list = reallocarray(list, list_size, sizeof(struct range));
    if (!list) {
      exit(1);
    }
  }
  list[list_one].start = s;
  list[list_one].end = e;
  list_one++;
}

uint64_t calc2() {
  uint64_t acc = 0;

  uint64_t end = 0;
  uint64_t min_start = 0;
  for (int i = list_one - 1; i >= 0; i--) {
    if (0 == end) {
      end = list[i].end;
      min_start = list[i].start;
    }

    for (int j = i; j >= 0; j--) {
      if (list[j].end < min_start) {
        acc += end - min_start + 1;
        end = list[j].end;
        if (min_start < end) {
          end = min_start;
        }
        min_start = list[j].start;
      } else if (list[j].start < min_start) {
        min_start = list[j].start;
      }
    }
  }
  acc += end - min_start + 1;
  return acc;
}

int compar(const void *_a, const void *_b) {
  const struct range *a = _a;
  const struct range *b = _b;

  if ((a->end) > (b->end)) {
    return 1;
  }
  if ((a->end) < (b->end)) {
    return -1;
  }
  return 0;
}

int main() {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  int stage = 0;

  char *line = NULL;
  for (;;) {
    size_t r;
    ssize_t length;
    if (-1 == (length = getline(&line, &r, stdin))) {
      break;
    }

    if (1 == strlen(line)) {
      stage = 1;
      continue;
    }

    if (1 == stage) {
      uint64_t ing;
      if (EOF == sscanf(line, "%" PRIu64, &ing)) {
        break;
      }

      if (!isspoiled_calc(ing)) {
        part1++;
      }
    } else if (0 == stage) {
      uint64_t st;
      uint64_t e;
      if (EOF == sscanf(line, "%" PRIu64 "-%" PRIu64, &st, &e)) {
        break;
      }

      add_list(st, e);
    }
  }

  qsort(list, list_one, sizeof(struct range), compar);
  part2 = calc2();

  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
  return 0;
}
