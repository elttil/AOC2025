#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _log10(int64_t number) {
  char tmp[256];
  snprintf(tmp, sizeof(tmp), "%lld", number);
  return strlen(tmp) - 1;
}

char *skip(char *s) {
  for (; *s && !isdigit(*s) && isspace(*s); s++)
    ;
  return s;
}

int main(int argc, char **argv) {
  uint64_t part1 = 0;
  uint64_t part2 = 0;

  char **lines = NULL;
  size_t lines_size = 0;
  size_t lines_num = 0;
  char *line = NULL;
  for (;;) {
    size_t r;
    ssize_t length;
    if (-1 == (length = getline(&line, &r, stdin))) {
      break;
    }

	if(lines_num >= lines_size) {
		lines_size += 4096;
		lines = reallocarray(lines, lines_size, sizeof(char*));
		if(!lines) return 1;
    }
    lines[lines_num] = strdup(line);
    lines_num++;
  }

  char *ops = lines[lines_num - 1];

  char *p = ops;

  for (int idx = 0; *p; idx++) {
    p = skip(p);
    char op = *p;
    p++;
    uint64_t acc = 0;
    for (int i = 0; i < lines_num - 1; i++) {
      char *l = lines[i];
      uint64_t t = 0;
      for (int j = 0; j < idx + 1; j++) {
        l = skip(l);
        if (0 == *l) {
          break;
        }
        t = strtoull(l, &l, 10);
      }

      if ('+' == op) {
        acc += t;
      } else if ('*' == op) {
        if (0 == acc) {
          acc = 1;
        }
        acc *= t;
      }
    }
    part1 += acc;
  }

  p = ops;

  for (int idx = 0; *p; idx++) {
    p = skip(p);
    char op = *p;
    if ('\0' == op) {
      break;
    }
    p++;

    int usual_start = -1;
    int max_log10 = 0;

    for (int i = 0; i < lines_num - 1; i++) {
      char *l = lines[i];
      uint64_t t = 0;
      char *s;
      for (int j = 0; j < idx + 1; j++) {
        l = skip(l);
        if (0 == *l) {
          break;
        }
		s = l;
        t = strtoll(l, &l, 10);
      }
      char *e = l;
      int lg = e - s;
      if (lg >= max_log10) {
        usual_start = s - lines[i];
        max_log10 = lg;
      }
    }
    max_log10--;

    int64_t acc = 0;
    int extra = 0;
    for (int k = 0; k < max_log10 + 1; k++) {
      int64_t tmp_acc = 0;
      for (int i = 0; i < lines_num - 1; i++) {
        char *l = lines[i];
        int64_t t = 0;
        int actual_start = -1;
        for (int j = 0; j < idx + 1; j++) {
          l = skip(l);
          if (0 == *l) {
            break;
          }
          actual_start = l - lines[i];
          t = strtoll(l, &l, 10);
        }

        int delta = actual_start - usual_start;
        for (; _log10(t) < max_log10 - delta; t *= 10)
          ;

        for (int j = 0; j < k; j++) {
          t /= 10;
        }
        t %= 10;

        if (0 == t) {
          extra++;
          continue;
        }
        if (extra > 0) {
          for (int x = 0; x < extra; x++) {
            tmp_acc *= 10;
          }
        }
        extra = 0;
        tmp_acc *= 10;
        tmp_acc += t;
      }
      if ('+' == op) {
        acc += tmp_acc;
      } else if ('*' == op) {
        if (0 == acc) {
          acc = 1;
        }
        acc *= tmp_acc;
      }
    }
    part2 += acc;
  }

  printf("part1: %" PRIu64 "\n", part1);
  printf("part2: %" PRIu64 "\n", part2);
  return 0;
}
