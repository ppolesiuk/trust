#include "settings.h"

#include <ctype.h>

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
  return PARSE_SIZE_OK;
}
