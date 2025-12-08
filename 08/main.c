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

struct circuit {
  struct box *boxes[4096];
  size_t num_boxes;
};

struct box {
  int64_t x;
  int64_t y;
  int64_t z;
  struct circuit *cir;
};

struct connection {
  struct box *a;
  struct box *b;
};

bool circuit_add(struct circuit *cir, struct box *box) {
  assert(!box->cir);
  box->cir = cir;

  for (size_t i = 0; i < cir->num_boxes; i++) {
    if (cir->boxes[i] == box) {
      return false;
    }
  }

  cir->boxes[cir->num_boxes] = box;
  cir->num_boxes++;
  return true;
}

struct circuit *circuits[4096 * 1000];
size_t num_circuits = 0;

void circuit_morph(struct circuit *a, struct circuit *b) {
  for (size_t i = 0; i < b->num_boxes; i++) {
    b->boxes[i]->cir = NULL;
    circuit_add(a, b->boxes[i]);
  }
  b->num_boxes = 0;
}

struct connection connections[1000 * 1000 * 10];
size_t num_connections = 0;

struct box boxes[4096 * 1000];
size_t num_boxes = 0;

static inline int64_t distance(struct box *a, struct box *b) {
  int64_t dx = a->x - b->x;
  int64_t dy = a->y - b->y;
  int64_t dz = a->z - b->z;
  return dx * dx + dy * dy + dz * dz;
}

static void add_connection(struct box *a, struct box *b) {
  connections[num_connections].a = a;
  connections[num_connections].b = b;
  num_connections++;
}

struct distance {
  struct box *a;
  struct box *b;
  int64_t distance;
};

struct distance dis[1000 * 1000 * 10];
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
      dis[num_distance].a = box;
      dis[num_distance].b = &boxes[j];
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

    int64_t x;
    int64_t y;
    int64_t z;
    sscanf(line, "%" PRIi64 ",%" PRIi64 ",%" PRIi64 "", &x, &y, &z);
    boxes[num_boxes].x = x;
    boxes[num_boxes].y = y;
    boxes[num_boxes].z = z;
    boxes[num_boxes].cir = NULL;
    num_boxes++;
  }

#ifdef VISUAL
  fp = ffmpeg_start(12, 1000, 1000, "test.mp4");
  assert(fp);
#endif // VISUAL

  sort();

  int biggest[3] = {-1, -1, -1};

  uint64_t sum = 1;

  for (size_t j = 0;; j++) {
    if (1000 == num_connections) {
      for (int i = 0; i < 3; i++) {
        uint64_t big = 0;
        for (int z = 0; z < (int)num_circuits; z++) {
          bool e = false;
          for (int k = 0; k < 3; k++) {
            if (z == biggest[k]) {
              e = true;
            }
          }
          if (e) {
            continue;
          }
          struct circuit *cir = circuits[z];
          if (cir->num_boxes > big) {
            big = cir->num_boxes;
            biggest[i] = z;
          }
        }
      }
      sum *= circuits[biggest[0]]->num_boxes;
      sum *= circuits[biggest[1]]->num_boxes;
      sum *= circuits[biggest[2]]->num_boxes;
    }
    if (j >= num_distance) {
      break;
    }
    struct box *a = dis[j].a;
    struct box *b = dis[j].b;

#ifdef VISUAL
    render(fp);
#endif // VISUAL

    add_connection(a, b);

    if (a->cir && a->cir == b->cir) {
      continue;
    }

    part2 = a->x * b->x;

    if (a->cir && !b->cir) {
      if (circuit_add(a->cir, b)) {
      }
      continue;
    }

    if (b->cir) {
      if (a->cir) {
        circuit_morph(a->cir, b->cir);
        continue;
      }
      if (circuit_add(b->cir, a)) {

        continue;
      }
    }

    struct circuit *cir = malloc(sizeof(struct circuit));
    cir->num_boxes = 0;
    circuits[num_circuits] = cir;
    num_circuits++;

    circuit_add(cir, a);
    circuit_add(cir, b);
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
