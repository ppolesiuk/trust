#include "settings.h"

#include <ctype.h>
#include <string.h>

static int parse_num_nc(const char *str, int *num, int min, int max) {
  int n = 0;
  for (; isdigit(*str); str++) {
    n = n*10 + *str - '0';
    if (n > max) {
      return CHECK_FAIL;
    }
  }
  if (n < min) {
    return CHECK_FAIL;
  }
  *num = n;
  return CHECK_OK;
}

int parse_number(const char *str, int *num, int min, int max) {
  for (int i = 0; str[i]; ++i) {
    if (!isdigit(*str)) {
      return CHECK_FAIL;
    }
  }
  return parse_num_nc(str, num, min, max);
}

static int check_size_fmt(const char *str, const char **size_y) {
  do {
    if (!isdigit(*str)) {
      return CHECK_FAIL;
    }
    str++;
  } while (*str != 'x');
  str++;
  *size_y = str;
  do {
    if (!isdigit(*str)) {
      return CHECK_FAIL;
    }
    str++;
  } while (*str != 0);
  return CHECK_OK;
}

parse_size_result_t parse_size(const char *str, settings_t *settings) {
  const char *size_y;
  if (check_size_fmt(str, &size_y)) {
    return PARSE_SIZE_SYNTAX_ERROR;
  }
  if (parse_num_nc(str, &settings->board_size_x, 1, MAX_BOARD_SIZE)) {
    return PARSE_SIZE_BAD_VALUE;
  }
  if (parse_num_nc(size_y, &settings->board_size_y, 1, MAX_BOARD_SIZE)) {
    return PARSE_SIZE_BAD_VALUE;
  }
  return (settings->board_size_x == 1 && settings->board_size_y == 1) ?
    PARSE_SIZE_TOO_SMALL : PARSE_SIZE_OK;
}

#define SERIALIZE_INT(x)   fprintf(file, #x "=%d\n", settings->x)
#define SERIALIZE_ULONG(x) fprintf(file, #x "=%lu\n", settings->x)
#define SERIALIZE_STRING(x) \
  if (settings->x == NULL) { \
    fprintf(file, #x "=-1|\n"); \
  } else { \
    fprintf(file, #x "=%d|%s\n", (int)strlen(settings->x), settings->x); \
  }

void settings_serialize(FILE *file, const settings_t *settings) {
  fprintf(file, "#SETTINGS\n");
  SERIALIZE_INT(board_size_x);
  SERIALIZE_INT(board_size_y);
  SERIALIZE_INT(state_n);
  SERIALIZE_INT(step_n);
  SERIALIZE_INT(turn_n);
  SERIALIZE_INT(play_area);
  SERIALIZE_INT(kill_area);
  SERIALIZE_INT(cross_area);
  SERIALIZE_INT(lifetime);
  SERIALIZE_INT(stat_report_rate);
  SERIALIZE_INT(stat_flush_rate);
  SERIALIZE_INT(example_rate);
  SERIALIZE_INT(image_rate);
  SERIALIZE_INT(flags);
  SERIALIZE_ULONG(seed);
  SERIALIZE_ULONG(mistake_rate);
  SERIALIZE_ULONG(cross_rate);
  SERIALIZE_ULONG(state_mut_rate);
  SERIALIZE_ULONG(action_mut_rate);
  SERIALIZE_ULONG(edge_mut_rate);
  SERIALIZE_STRING(stat_file);
  SERIALIZE_STRING(example_name);
  SERIALIZE_STRING(image_name);
}
