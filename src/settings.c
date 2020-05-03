#include "settings.h"

#include "serialization.h"

#include <ctype.h>
#include <limits.h>

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

void settings_serialize(FILE *file, const settings_t *settings) {
  serialize_tag(file, "SETTINGS");
  SERIALIZE_INT(file, settings, board_size_x);
  SERIALIZE_INT(file, settings, board_size_y);
  SERIALIZE_INT(file, settings, state_n);
  SERIALIZE_INT(file, settings, step_n);
  SERIALIZE_INT(file, settings, turn_n);
  SERIALIZE_INT(file, settings, play_area);
  SERIALIZE_INT(file, settings, kill_area);
  SERIALIZE_INT(file, settings, cross_area);
  SERIALIZE_INT(file, settings, lifetime);
  SERIALIZE_INT(file, settings, stat_report_rate);
  SERIALIZE_INT(file, settings, stat_flush_rate);
  SERIALIZE_INT(file, settings, example_rate);
  SERIALIZE_INT(file, settings, image_rate);
  SERIALIZE_INT(file, settings, backup_rate);
  SERIALIZE_INT(file, settings, flags);
  SERIALIZE_ULONG(file, settings, seed);
  SERIALIZE_ULONG(file, settings, mistake_rate);
  SERIALIZE_ULONG(file, settings, cross_rate);
  SERIALIZE_ULONG(file, settings, state_mut_rate);
  SERIALIZE_ULONG(file, settings, action_mut_rate);
  SERIALIZE_ULONG(file, settings, edge_mut_rate);
  SERIALIZE_STRING(file, settings, stat_file);
  SERIALIZE_STRING(file, settings, example_name);
  SERIALIZE_STRING(file, settings, image_name);
}

void settings_deserialize(FILE *file, settings_t *settings) {
  deserialize_tag(file, "SETTINGS");
  DESERIALIZE_INT(file, settings, board_size_x, 1, MAX_BOARD_SIZE);
  DESERIALIZE_INT(file, settings, board_size_y, 1, MAX_BOARD_SIZE);
  DESERIALIZE_INT(file, settings, state_n, 1, MAX_STATE_N);
  DESERIALIZE_INT(file, settings, step_n, 0, MAX_STEP_N);
  DESERIALIZE_INT(file, settings, turn_n, 1, MAX_TURN_N);
  DESERIALIZE_INT(file, settings, play_area, 1, MAX_AREA_SIZE);
  DESERIALIZE_INT(file, settings, kill_area, 1, MAX_AREA_SIZE);
  DESERIALIZE_INT(file, settings, cross_area, 1, MAX_AREA_SIZE);
  DESERIALIZE_INT(file, settings, lifetime, 1, MAX_LIFETIME);
  DESERIALIZE_INT(file, settings, stat_report_rate, 1, MAX_REPORT_RATE);
  DESERIALIZE_INT(file, settings, stat_flush_rate, 1, MAX_REPORT_RATE);
  DESERIALIZE_INT(file, settings, example_rate, 1, MAX_REPORT_RATE);
  DESERIALIZE_INT(file, settings, image_rate, 1, MAX_REPORT_RATE);
  DESERIALIZE_INT(file, settings, backup_rate, 1, MAX_REPORT_RATE);
  DESERIALIZE_INT(file, settings, flags, 0, INT_MAX);
  DESERIALIZE_ULONG(file, settings, seed, 0, ULONG_MAX);
  DESERIALIZE_ULONG(file, settings, mistake_rate, 0, ULONG_MAX);
  DESERIALIZE_ULONG(file, settings, cross_rate, 0, ULONG_MAX);
  DESERIALIZE_ULONG(file, settings, state_mut_rate, 0, ULONG_MAX);
  DESERIALIZE_ULONG(file, settings, action_mut_rate, 0, ULONG_MAX);
  DESERIALIZE_ULONG(file, settings, edge_mut_rate, 0, ULONG_MAX);
  DESERIALIZE_STRING(file, settings, stat_file);
  DESERIALIZE_STRING(file, settings, example_name);
  DESERIALIZE_STRING(file, settings, image_name);
}
