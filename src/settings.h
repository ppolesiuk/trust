#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define MAX_BOARD_SIZE 4096
#define MAX_STATE_N    10000
#define MAX_STEP_N     200000000

#define CHECK_OK   0
#define CHECK_FAIL 1

typedef struct settings {
  int         board_size_x;
  int         board_size_y;
  int         state_n;
  int         step_n;
  int         turn_n;
  int         play_area;
  int         kill_area;
  int         cross_area;
  int         stat_report_step;
  int         stat_flush_step;
  int         aexample_step;
  const char *stat_file;
  const char *automaton_file;
} settings_t;

int parse_number(const char *str, int *num, int min, int max);

typedef enum parse_size_result {
  PARSE_SIZE_OK,
  PARSE_SIZE_SYNTAX_ERROR,
  PARSE_SIZE_BAD_VALUE
} parse_size_result_t;

parse_size_result_t parse_size(const char *str, settings_t *settings);

#endif
