#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//#define VISUAL

struct grid {
  char *grid;
  int max_x;
  int max_y;
};

static inline char grid_get(struct grid *grid, int x, int y) {
  if (x < 0 || x >= grid->max_x) {
    return 0;
  }
  if (y < 0 || y >= grid->max_x) {
    return 0;
  }
  return grid->grid[y * grid->max_x + x];
}

static inline void grid_set(struct grid *grid, int x, int y, char c) {
  if (x < 0 || x >= grid->max_x) {
    return;
  }
  if (y < 0 || y >= grid->max_x) {
    return;
  }
  grid->grid[y * grid->max_x + x] = c;
}

struct line {
  size_t x;
  size_t y;
  size_t timelines;
  bool dead;
};

struct line lines[4096 * 8];
size_t num_lines = 0;
size_t num_splits = 0;

bool line_add(size_t x, size_t y, size_t timelines);

bool line_exists(size_t x, size_t y, bool remove, bool extra,
                 size_t timelines) {
  for (size_t i = 0; i < num_lines; i++) {
    if (lines[i].dead) {
      continue;
    }
    if (x == lines[i].x) {
      assert(y >= lines[i].y);
      if (extra) {
        lines[i].timelines += timelines;
        return true;
      }
      if (remove) {
        if (!line_exists(x - 1, y, false, true, lines[i].timelines)) {
          lines[i].x--;
          lines[i].y = y;
          (void)line_add(x + 1, y, lines[i].timelines);
          num_splits++;
        } else if (!line_exists(x + 1, y, false, true, lines[i].timelines)) {
          lines[i].x++;
          lines[i].y = y;
          num_splits++;
        } else {
          num_splits++;
          lines[i].dead = true;
        }
      }
      return true;
    }
  }
  return false;
}

bool line_add(size_t x, size_t y, size_t timelines) {
  if (line_exists(x, y, false, true, timelines)) {
    return false;
  }
  lines[num_lines].x = x;
  lines[num_lines].y = y;
  lines[num_lines].dead = false;
  lines[num_lines].timelines = timelines;
  num_lines++;
  return true;
}

#ifdef VISUAL
FILE *fp = NULL;

void render(FILE *fp, struct grid *grid, size_t frame) {
  for (int y = 0; y < grid->max_y; y++) {
    for (int x = 0; x < grid->max_x; x++) {
      uint8_t rgb[3] = {0, 0, 0};
      char c = grid_get(grid, x, y);
      if ('S' == c) {
        rgb[0] = 0xFF;
        rgb[1] = 0xFF;
      }
      if ('^' == c) {
        rgb[0] = 0xFF;
        rgb[1] = 0xFF;
        rgb[2] = 0xFF;
      }

      if (y <= frame) {
        for (int i = 0; i < num_lines; i++) {
          if (lines[i].y > y) {
            continue;
          }
          if (lines[i].x != x) {
            continue;
          }
          if (lines[i].dead) {
            continue;
          }
          rgb[0] = 0xFF;
        }
      }

      fwrite(rgb, 1, sizeof(rgb), fp);
    }
  }
}
#endif // VISUAL

uint64_t calc(struct grid *grid) {
  uint64_t result = 0;
  for (int y = 0; y < grid->max_y; y++) {

#ifdef VISUAL
    render(fp, grid, y);
#endif // VISUAL

    for (int x = 0; x < grid->max_x; x++) {
      char c = grid_get(grid, x, y);
      if ('^' != c && 'S' != c) {
        continue;
      }
      if ('S' == c) {
        line_add(x, y, 1);
        continue;
      }
      if ('^' == c) {

        if (!line_exists(x, y, true, false, 1)) {
          continue;
        }
        continue;
      }
    }
  }
  return num_splits;
}

#define FFMPEG_BIN "ffmpeg"

FILE *ffmpeg_start(unsigned int fps, unsigned int x, unsigned int y,
                   char *output) {
  char cmd[512];
  snprintf(cmd, sizeof(cmd),
           "%s -y -f rawvideo -s %ux%u -pix_fmt rgb24 -r %u -i - -an %s -f vp9",
           FFMPEG_BIN, x, y, fps, output);

  return popen(cmd, "w");
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
      buffer_size = offset + length + buffer_size;
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

#ifdef VISUAL
  fp = ffmpeg_start(12, grid.max_x, grid.max_y, "out.webm");
#endif // VISUAL

  part1 = calc(&grid);
  for (size_t i = 0; i < num_lines; i++) {
    if (lines[i].dead) {
      continue;
    }
    part2 += lines[i].timelines;
  }

#ifdef VISUAL
  fflush(fp);
  pclose(fp);
#endif // VISUAL

  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
  wait(NULL);
  return 0;
}
