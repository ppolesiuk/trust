#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <stdio.h>

#define MAX_BOARD_SIZE  4096
#define MAX_AREA_SIZE   2048
#define MAX_STATE_N     10000
#define MAX_STEP_N      200000000
#define MAX_TURN_N      1000000
#define MAX_LIFETIME    10000
#define MAX_REPORT_RATE 1000000

#define CHECK_OK   0
#define CHECK_FAIL 1

#define F_QUIET            0x1
#define F_SPECIES_MAP      0x2
#define F_SHOW_UNREACHABLE 0x4
#define F_DETERMINISTIC    0x10
#define F_MISTAKE_AWARE    0x20
#define F_MOVE_AWARE       0x40

typedef struct settings {
  int           board_size_x;
  int           board_size_y;
  int           state_n;
  int           step_n;
  int           turn_n;
  int           play_area;
  int           kill_area;
  int           cross_area;
  int           lifetime;
  int           stat_report_rate;
  int           stat_flush_rate;
  int           example_rate;
  int           image_rate;
  int           flags;
  unsigned long seed;
  unsigned long mistake_rate;
  unsigned long cross_rate;
  unsigned long state_mut_rate;
  unsigned long action_mut_rate;
  unsigned long edge_mut_rate;
  const char   *stat_file;
  const char   *example_name;
  const char   *image_name;
} settings_t;

int parse_number(const char *str, int *num, int min, int max);

typedef enum parse_size_result {
  PARSE_SIZE_OK,
  PARSE_SIZE_SYNTAX_ERROR,
  PARSE_SIZE_BAD_VALUE,
  PARSE_SIZE_TOO_SMALL
} parse_size_result_t;

parse_size_result_t parse_size(const char *str, settings_t *settings);

void settings_serialize(FILE *file, const settings_t *settings);

#endif
