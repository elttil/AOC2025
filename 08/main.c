#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

// #define VISUAL

#ifdef VISUAL
FILE *fp = NULL;
#endif // VISUAL

struct element {
  uint64_t parent;
  size_t size;
};

struct dsu {
  struct element *el;
  size_t el_size;
};

bool dsu_init(struct dsu *ctx, size_t size) {
  ctx->el_size = 0;
  ctx->el = malloc(size * sizeof(struct element));
  if (!ctx->el) {
    return false;
  }
  ctx->el_size = size;
  for (size_t i = 0; i < size; i++) {
    ctx->el[i].parent = i;
    ctx->el[i].size = 1;
  }
  return true;
}

uint64_t dsu_find(struct dsu *ctx, uint64_t idx) {
  uint64_t parent = ctx->el[idx].parent;
  if (idx == parent) {
    return idx;
  }
  uint64_t ret_parent = dsu_find(ctx, parent);
  ctx->el[idx].parent = ret_parent;
  return ret_parent;
}

uint64_t dsu_union(struct dsu *ctx, uint64_t a, uint64_t b) {
  uint64_t a_parent = dsu_find(ctx, a);
  uint64_t b_parent = dsu_find(ctx, b);
  if (a_parent == b_parent) {
    return 0;
  }
  if (ctx->el[a_parent].size > ctx->el[b_parent].size) {
    ctx->el[b_parent].parent = a_parent;
    ctx->el[a_parent].size += ctx->el[b_parent].size;
    return ctx->el[a_parent].size;
  } else {
    ctx->el[a_parent].parent = b_parent;
    ctx->el[b_parent].size += ctx->el[a_parent].size;
    return ctx->el[b_parent].size;
  }
}

struct box {
  int64_t x;
  int64_t y;
  int64_t z;
};

size_t num_connections = 0;
#ifdef VISUAL
struct connection {
  struct box *a;
  struct box *b;
};

struct connection *connections = NULL;
size_t connections_size = 0;
#endif // VISUAL

struct box *boxes = NULL;
size_t boxes_size = 0;
size_t num_boxes = 0;

static inline int64_t distance(struct box *a, struct box *b) {
  int64_t dx = a->x - b->x;
  int64_t dy = a->y - b->y;
  int64_t dz = a->z - b->z;
  return dx * dx + dy * dy + dz * dz;
}

static inline void add_connection(struct box *a, struct box *b) {
#ifdef VISUAL
  if (num_connections >= connections_size) {
    connections_size += 4096 * 1000;
    connections = realloc(connections, connections_size);
    assert(connections);
  }

  connections[num_connections].a = a;
  connections[num_connections].b = b;
#endif // VISUAL
  num_connections++;
}

struct distance {
  uint64_t distance;
  uint32_t i;
  uint32_t j;
  //  struct box *a;
  //  struct box *b;
};

struct distance *dis = NULL;
size_t distance_size = 0;
size_t num_distance = 0;

int compar(const void *_a, const void *_b) {
  const struct distance *a = _a;
  const struct distance *b = _b;
  if (a->distance < b->distance) {
    return -1;
  }
  if (a->distance > b->distance) {
    return 1;
  }
  return 0;
}

void sort(void) {
  for (size_t i = 0; i < num_boxes; i++) {
    struct box *box = &boxes[i];
    for (size_t j = i + 1; j < num_boxes; j++) {
      if (i == j) {
        continue;
      }
      if (num_distance >= distance_size) {
        distance_size += 4096 * 1000;
        dis = realloc(dis, distance_size * sizeof(struct distance));
        assert(dis);
      }

      dis[num_distance].i = i;
      dis[num_distance].j = j;
      dis[num_distance].distance = distance(box, &boxes[j]);
      num_distance++;
    }
  }
  qsort(dis, num_distance, sizeof(struct distance), compar);
}

#ifdef VISUAL
#define FFMPEG_BIN "ffmpeg"

FILE *ffmpeg_start(unsigned int fps, unsigned int x, unsigned int y,
                   char *output) {
  char cmd[512];
  snprintf(cmd, sizeof(cmd),
           "%s -y -f rawvideo -hwaccel cuda -s %ux%u -pix_fmt rgb24 -r %u -i - "
           "-an %s",
           FFMPEG_BIN, x, y, fps, output);

  return popen(cmd, "w");
}

static inline void place_pixel(char *buffer, int x, int y, uint8_t r, uint8_t g,
                               uint8_t b) {
  uint32_t offset = (x + y * 1000) * sizeof(uint8_t[3]);
  uint8_t rgb[3] = {r, g, b};
  memcpy(buffer + offset, rgb, sizeof(rgb));
}

void swap(int *a, int *b) {
  int tmp = *b;
  *b = *a;
  *a = tmp;
}

float _abs(float a) {
  if (a < 0.0f) {
    return -1.0f * a;
  }
  return a;
}

static inline void draw_line(char *buffer, int x0, int y0, int x1, int y1,
                             uint8_t r, uint8_t g, uint8_t b) {
  bool steep = false;
  if (abs(x0 - x1) < abs(y0 - y1)) {
    swap(&x0, &y0);
    swap(&x1, &y1);
    steep = true;
  }
  if (x0 > x1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }
  int dx = x1 - x0;
  int dy = y1 - y0;
  float derror = _abs((float)dy / ((float)dx));
  float error = 0;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    if (steep) {
      place_pixel(buffer, y, x, r, g, b);
    } else {
      place_pixel(buffer, x, y, r, g, b);
    }
    error += derror;
    if (error > .5) {
      y += (y1 > y0 ? 1 : -1);
      error -= 1.;
    }
  }
}

void render(FILE *fp) {
  char *buffer = calloc(1000 * 1000, sizeof(uint8_t[3]));
  assert(buffer);
  for (int i = 0; i < num_boxes; i++) {
    place_pixel(buffer, boxes[i].x, boxes[i].y, 0xFF, 0xFF, 0xFF);
  }
  for (int i = 0; i < num_connections; i++) {
    struct box *a = connections[i].a;
    struct box *b = connections[i].b;

    draw_line(buffer, a->x, a->y, b->x, b->y, 0x00, 0xFF, 0x00);
  }
  fwrite(buffer, 1000 * 1000, sizeof(uint8_t[3]), fp);
  free(buffer);
}

#endif // VISUAL

int main() {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  char *line = NULL;
  for (;;) {
    size_t r;
    ssize_t length;
    if (-1 == (length = getline(&line, &r, stdin))) {
      assert(ENOMEM != errno);
      break;
    }

    if (num_boxes >= boxes_size) {
      boxes_size += 4096;
      boxes = realloc(boxes, boxes_size * sizeof(struct box));
      assert(boxes);
    }

    int64_t x;
    int64_t y;
    int64_t z;
    sscanf(line, "%" PRIi64 ",%" PRIi64 ",%" PRIi64 "", &x, &y, &z);
    boxes[num_boxes].x = x;
    boxes[num_boxes].y = y;
    boxes[num_boxes].z = z;
    num_boxes++;
    //	printf("Line: %llu %llu %llu\n", x, y, z);
  }

#ifdef VISUAL
  fp = ffmpeg_start(12, 1000, 1000, "test.mp4");
  assert(fp);
#endif // VISUAL

  sort();

  int biggest[3] = {-1, -1, -1};

  uint64_t sum = 1;

  struct dsu ctx;
  dsu_init(&ctx, num_boxes);

  for (size_t j = 0;; j++) {
    if (1000 == num_connections) {
      for (int i = 0; i < 3; i++) {
        uint64_t big = 0;
        for (int z = 0; z < (int)ctx.el_size; z++) {
          bool e = false;
          for (int k = 0; k < 3; k++) {
            if (z == biggest[k]) {
              e = true;
            }
          }
          if (e) {
            continue;
          }
          if ((int)ctx.el[z].size > big) {
            big = ctx.el[z].size;
            biggest[i] = z;
          }
        }
      }
      sum *= ctx.el[biggest[0]].size;
      sum *= ctx.el[biggest[1]].size;
      sum *= ctx.el[biggest[2]].size;
    }
    if (j >= num_distance) {
      break;
    }
    struct box *a = &boxes[dis[j].i];
    struct box *b = &boxes[dis[j].j];

    add_connection(a, b);
#ifdef VISUAL
    render(fp);

    if (part2 == 0 && num_boxes == dsu_union(&ctx, dis[j].i, dis[j].j)) {
      part2 = a->x * b->x;
    }
#else  // VISUAL
    if (num_boxes == dsu_union(&ctx, dis[j].i, dis[j].j)) {
      part2 = a->x * b->x;
      break;
    }
#endif // VISUAL
  }
  part1 = sum;

#ifdef VISUAL
  fflush(fp);
  pclose(fp);
#endif // VISUAL

  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
#ifdef VISUAL
  wait(NULL);
#endif // VISUAL
  return 0;
}
