#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct grid {
  char *grid;
  int max_x;
  int max_y;
};

char grid_get(struct grid *grid, int x, int y) {
  if (x < 0 || x >= grid->max_x) {
    return 0;
  }
  if (y < 0 || y >= grid->max_x) {
    return 0;
  }
  return grid->grid[y * grid->max_x + x];
}

void grid_set(struct grid *grid, int x, int y, char c) {
  if (x < 0 || x >= grid->max_x) {
    return;
  }
  if (y < 0 || y >= grid->max_x) {
    return;
  }
  grid->grid[y * grid->max_x + x] = c;
}

uint64_t calc(struct grid *grid) {
  uint64_t result = 0;
  for (int y = 0; y < grid->max_y; y++) {
    for (int x = 0; x < grid->max_x; x++) {
      if ('@' != grid_get(grid, x, y)) {
        continue;
      }
      int count = 0;
      for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
          char c = grid_get(grid, x + i, y + j);
          if (i == 0 && j == 0) {
            continue;
          }
          if ('@' == c || 'x' == c) {
            count++;
          }
        }
      }
      if (count < 4) {
        grid_set(grid, x, y, 'x');
        result++;
      }
    }
  }
  return result;
}

uint64_t purge(struct grid *grid) {
  uint64_t result = 0;
  for (int y = 0; y < grid->max_y; y++) {
    for (int x = 0; x < grid->max_x; x++) {
      if ('x' == grid_get(grid, x, y)) {
        grid_set(grid, x, y, '.');
        result++;
      }
    }
  }
  return result;
}

int main() {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  size_t buffer_size = 0x1000 << 4;
  char *map = malloc(buffer_size);
  assert(map);
  size_t offset = 0;
  size_t n_lines = 0;

  size_t max_x = 0;

  char *line = NULL;
  for (;;) {
    size_t r;
    ssize_t length;
    if (-1 == (length = getline(&line, &r, stdin))) {
      assert(ENOMEM != errno);
      break;
    }
    length--;
    if (offset + length > buffer_size) {
      buffer_size = offset + length + 0x1000;
      map = realloc(map, buffer_size);
      assert(map);
    }
    memcpy(map + offset, line, length);
    n_lines++;
    offset += length;
    max_x = length;
  }

  struct grid grid = {
      .grid = map,
      .max_x = max_x,
      .max_y = n_lines,
  };

  part1 = calc(&grid);

  for (;;) {
    uint64_t r = purge(&grid);
    if (0 == r) {
      break;
    }
    part2 += r;
    calc(&grid);
  }

  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
  return 0;
}
