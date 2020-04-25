#include <argp.h>
#include <stdio.h>
#include <assert.h>

#define STR_(x) #x
#define STR(x) STR_(x)

#define DFLT_BOARD_SIZE       32
#define DFLT_STATES           32
#define DFLT_STEPS            10000
#define DFLT_TURNS            16
#define DFLT_PLAY_AREA        3
#define DFLT_KILL_AREA        2
#define DFLT_CROSS_AREA       4
#define DFLT_STAT_REPORT_STEP 1
#define DFLT_STAT_FLUSH_STEP  100
#define DFLT_AEXAMPLE_STEP    200
#define DFLT_STAT_FILE        "stat"
#define DFLT_AUTOMATON_FILE   "automaton_"

#include "settings.h"
#include "world.h"

/* ========================================================================= */
/* Argument parsing */

const char *argp_program_version = "trust 0.1";
static const char doc[] =
  "Evolution of trust";

static struct argp_option options[] =
  { { "board-size", 'b', "SIZE", 0,
      "Specify the size of the board (default is "
      STR(DFLT_BOARD_SIZE) "x" STR(DFLT_BOARD_SIZE) ")." }
  , { "states", 's', "STATES", 0,
      "Specify the number of automaton states (default is "
      STR(DFLT_STATES) ")." }
  , { "steps",  'n', "STEPS", 0,
      "Specify the number of steps "
      "(default is " STR(DFLT_STEPS) ", 0 means run indefinitely)." }
  , { 0 }
  };

static void parse_size_opt(char *arg, struct argp_state *state);

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  settings_t *settings = state->input;
  switch (key) {
  case 'b':
    parse_size_opt(arg, state);
    break;
  case 's':
    if (parse_number(arg, &settings->state_n, 1, MAX_STATE_N) != CHECK_OK) {
      argp_error(state, "Number of states must be in range beetwen 1 and "
        STR(MAX_STATE_N) ".");
    }
    break;
  case 'n':
    if (parse_number(arg, &settings->step_n, 0, MAX_STEP_N) != CHECK_OK) {
      argp_error(state, "Number of steps must be in range beetwen 0 and "
        STR(MAX_STEP_N) ".");
    }
    break;
  case ARGP_KEY_ARG:
    argp_usage(state);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static void parse_size_opt(char *arg, struct argp_state *state) {
  switch (parse_size(arg, state->input)) {
  case PARSE_SIZE_OK:
    break;
  case PARSE_SIZE_SYNTAX_ERROR:
    argp_error(state, "Invalid board size. "
      "It should be in <SIZE_X>x<SIZE_Y> format, e.g., 13x24.");
    break;
  case PARSE_SIZE_BAD_VALUE:
    argp_error(state, "Board size must be in range beetwen 1 and "
      STR(MAX_BOARD_SIZE) ".");
    break;
  default:
    assert(0);
  }
}

static struct argp argp = { options, parse_opt, 0, doc, 0, 0, 0 };

int main(int argc, char **argv) {
  world_t world =
    { .settings = 
      { .board_size_x     = DFLT_BOARD_SIZE
      , .board_size_y     = DFLT_BOARD_SIZE
      , .state_n          = DFLT_STATES
      , .step_n           = DFLT_STEPS
      , .turn_n           = DFLT_TURNS
      , .play_area        = DFLT_PLAY_AREA
      , .kill_area        = DFLT_KILL_AREA
      , .cross_area       = DFLT_CROSS_AREA
      , .stat_report_step = DFLT_STAT_REPORT_STEP
      , .stat_flush_step  = DFLT_STAT_FLUSH_STEP
      , .aexample_step    = DFLT_AEXAMPLE_STEP
      , .stat_file        = DFLT_STAT_FILE
      , .automaton_file   = DFLT_AUTOMATON_FILE
      }
    };

  argp_parse(&argp, argc, argv, 0, 0, &world.settings);

  world_init(&world);
  do {
    world_reset(&world);
    world_play(&world);
    world_kill_weak(&world);
    world_spawn_new(&world);
    world_report(&world);
    printf("\r%lu", world.step);
    fflush(stdout);
  } while (world_next_step(&world));

  world_destroy(&world);
  return 0;
}
