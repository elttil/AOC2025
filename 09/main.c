#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct edge {
  int64_t x;
  int64_t y;
};

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

struct edge edges[4096 * 100];
size_t num_edges = 0;

struct box {
  struct edge *edges[2];
  uint64_t area;
};

struct wall {
  struct edge *edges[2];
};

struct wall walls[4096 * 100];
size_t num_walls = 0;

struct box boxes[4096 * 100];
size_t num_boxes = 0;

static bool check_wall(struct wall *wall, int x, int y, bool x_axis,
                       bool sign) {
  struct edge *a = wall->edges[0];
  struct edge *b = wall->edges[1];
  if (x_axis) {
    uint64_t start_y = min(a->y, b->y);
    uint64_t end_y = max(a->y, b->y);
    if (y >= start_y && y <= end_y) {
      if (sign) {
        if (x <= a->x) {
          return true;
        }
      } else {
        if (x >= a->x) {
          return true;
        }
      }
      return false;
    }
  } else {
    uint64_t start_x = min(a->x, b->x);
    uint64_t end_x = max(a->x, b->x);
    if (x >= start_x && x <= end_x) {
      if (sign) {
        if (y <= a->y) {
          return true;
        }
      } else {
        if (y >= a->y) {
          return true;
        }
      }
    }
  }
  return false;
}

static inline bool find_wall(int x, int y, bool x_axis, bool sign, int *ptr) {
  if (-1 != *ptr) {
    if (check_wall(&walls[*ptr], x, y, x_axis, sign)) {
      return true;
    }
  }
  for (size_t i = 0; i < num_walls; i++) {
    if (check_wall(&walls[i], x, y, x_axis, sign)) {
      *ptr = i;
      return true;
    }
  }
  *ptr = -1;
  return false;
}

int prev[] = {-1, -1, -1, -1};

static inline bool valid_pos(uint64_t x, uint64_t y, bool *done) {
  if (!find_wall(x, y, false, false, &prev[0])) {
    return false;
  }
  if (!find_wall(x, y, false, true, &prev[1])) {
    return false;
  }
  if (!find_wall(x, y, true, false, &prev[2])) {
    return false;
  }
  if (!find_wall(x, y, true, true, &prev[3])) {
    return false;
  }
  return true;
}

static bool walk(struct edge a, struct edge b) {
  int x_inc = 0;
  int y_inc = 0;
  if (a.x == b.x) {
    if (a.y > b.y) {
      y_inc = -1;
    } else {
      y_inc = 1;
    }
  }
  if (a.y == b.y) {
    if (a.x > b.x) {
      x_inc = -1;
    } else {
      x_inc = 1;
    }
  }

  int prev_wall = -1;

  bool done = false;
  for (; !(a.x == b.x && a.y == b.y);) {
    if (!valid_pos(a.x, a.y, &done)) {
      return false;
    }
    a.x += x_inc;
    a.y += y_inc;
  }
  return true;
}

static bool is_valid(struct box *box) {
  struct edge *a = box->edges[0];
  struct edge *b = box->edges[1];

  struct edge c;
  c.x = b->x;
  c.y = a->y;

  struct edge d;
  d.x = a->x;
  d.y = b->y;

  if (!walk(*a, c)) {
    return false;
  }
  if (!walk(*a, d)) {
    return false;
  }
  if (!walk(c, *b)) {
    return false;
  }
  if (!walk(d, *b)) {
    return false;
  }
  return true;
}

uint64_t calc_area(struct edge *a, struct edge *b) {
  uint64_t side_x = 0;
  uint64_t side_y = 0;
  if (a->x < b->x) {
    side_x = b->x - a->x + 1;
  } else {
    side_x = a->x - b->x + 1;
  }
  if (a->y < b->y) {
    side_y = b->y - a->y + 1;
  } else {
    side_y = a->y - b->y + 1;
  }
  return side_x * side_y;
}

int compar(const void *_a, const void *_b) {
  const struct box *a = _a;
  const struct box *b = _b;
  if (a->area < b->area) {
    return -1;
  }
  if (a->area > b->area) {
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  for (;;) {
    int64_t x;
    int64_t y;
    if (EOF == scanf("%" PRIi64 ",%" PRIi64, &x, &y)) {
      break;
    }
    scanf("");
    edges[num_edges].x = x;
    edges[num_edges].y = y;
    num_edges++;
  }

  for (int i = 0; i < num_edges; i++) {
    for (int j = i + 1; j < num_edges; j++) {
      boxes[num_boxes].edges[0] = &edges[i];
      boxes[num_boxes].edges[1] = &edges[j];
      boxes[num_boxes].area = calc_area(&edges[i], &edges[j]);
      num_boxes++;
    }
  }
  qsort(boxes, num_boxes, sizeof(struct box), compar);
  struct box *f = &boxes[num_boxes - 1];
  part1 = calc_area(f->edges[0], f->edges[1]);

  for (int i = 0; i < num_edges; i++) {
    for (int j = i + 1; j < num_edges; j++) {
      if (edges[i].x != edges[j].x && edges[i].y != edges[j].y) {
        continue;
      }
      walls[num_walls].edges[0] = &edges[i];
      walls[num_walls].edges[1] = &edges[j];
      num_walls++;
    }
  }

  for (int i = num_boxes - 1; i >= 0; i--) {
    struct box *t = &boxes[i];
    if (is_valid(t)) {
      part2 = calc_area(t->edges[0], t->edges[1]);
      break;
    }
  }

  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
  return 0;
}
