#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct FileMap {
  void *addr;
  size_t len;
  int fd;
};

int file_map(const char *file, struct FileMap *map) {
  int size;
  struct stat s;
  map->fd = open(file, O_RDONLY);

  if (-1 == fstat(map->fd, &s)) {
    return 0;
  }
  size = s.st_size;

  void *rc = mmap(0, size, PROT_READ, MAP_PRIVATE, map->fd, 0);
  if (MAP_FAILED == rc) {
    return 0;
  }
  map->addr = rc;
  map->len = size;
  return 1;
}

int main(int argc, char **argv) {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  const char *file = (argc > 1) ? argv[1] : "input.txt";
  struct FileMap map;
  file_map(file, &map);
  char *str = map.addr;

  int64_t dial = 50;
  char *end = (char *)map.addr + map.len;
  for (; str < end;) {
    char d = *str;
    str++;
    int64_t number = strtoll(str, &str, 10);
    str++;
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
