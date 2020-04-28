#include <argp.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define STR_(x) #x
#define STR(x) STR_(x)

#define DFLT_BOARD_SIZE          32
#define DFLT_STATES              32
#define DFLT_STEPS               0
#define DFLT_TURNS               16
#define DFLT_PLAY_AREA           3
#define DFLT_KILL_AREA           2
#define DFLT_CROSS_AREA          4
#define DFLT_LIFETIME            2000
#define DFLT_STAT_REPORT_RATE    1
#define DFLT_STAT_FLUSH_RATE     10
#define DFLT_EXAMPLE_RATE        200
#define DFLT_IMAGE_RATE          10
#define DFLT_SEED                1337
#define DFLT_MISTAKE_RATE        0.0
#define DFLT_CROSS_RATE          1.0
#define DFLT_STATE_MUT_RATE      0.01
#define DFLT_ACTION_MUT_RATE     0.01
#define DFLT_EFGE_MUT_RATE       0.01
#define DFLT_STAT_FILE           NULL
#define DFLT_EXAMPLE_NAME        NULL
#define DFLT_IMAGE_NAME          NULL

#include "settings.h"
#include "world.h"

/* ========================================================================= */
/* Argument parsing */

const char *argp_program_version = "trust 0.5.0";
static const char doc[] =
  "Evolution of trust";

#define OPT_BOARD_SIZE          'b'
#define OPT_STATES              's'
#define OPT_STEPS               'n'
#define OPT_TURNS               't'
#define OPT_PLAY_AREA           'P'
#define OPT_KILL_AREA           'K'
#define OPT_CROSS_AREA          'C'
#define OPT_LIFETIME            'l'
#define OPT_STAT_FILE           'o'
#define OPT_STAT_REPORT_RATE    'O'
#define OPT_EXAMPLE_NAME        'x'
#define OPT_EXAMPLE_RATE        'X'
#define OPT_IMAGE_NAME          'i'
#define OPT_IMAGE_RATE          'I'
#define OPT_QUIET               'q'
#define OPT_SPECIES_MAP         'M'
#define OPT_DETERMINISTIC       'd'
#define OPT_MISTAKE_AWARE       'a'
#define OPT_MOVE_AWARE          'A'
#define OPT_MISTAKE_RATE        'm'
#define OPT_CROSS_RATE          'c'
#define OPT_STATE_MUT_RATE      'S'
#define OPT_ACTION_MUT_RATE     'T'
#define OPT_EDGE_MUT_RATE       'E'
#define OPT_SHOW_UNREACHABLE    'u'

#define OPT_STAT_FLUSH_RATE  128
#define OPT_NO_SPECIES_MAP   129
#define OPT_SEED             130

static struct argp_option options[] =
  { { "board-size", OPT_BOARD_SIZE, "SIZE", 0,
      "Specify the size of the board (default is "
      STR(DFLT_BOARD_SIZE) "x" STR(DFLT_BOARD_SIZE) ")" }
  , { "states", OPT_STATES, "STATES", 0,
      "Specify the number of automaton states (default is "
      STR(DFLT_STATES) ")" }
  , { "steps", OPT_STEPS, "STEPS", 0,
      "Limit the number of simulation steps "
      "(0 is default and means to run indefinitely)" }
  , { "turns", OPT_TURNS, "TURNS", 0,
      "Specify the number of turns per game "
      "(default is " STR(DFLT_TURNS) ")" }
  , { "play-area", OPT_PLAY_AREA, "SIZE", 0,
      "Specify the size of area, where opponents are searched for "
      "(default is " STR(DFLT_PLAY_AREA) ")" }
  , { "kill-area", OPT_KILL_AREA, "SIZE", 0,
      "Specify the minimal distance between two eliminated automatons "
      "(default is " STR(DFLT_KILL_AREA) ")" }
  , { "cross-area", OPT_CROSS_AREA, "SIZE", 0,
      "Specify the size of area, from which parents of the new automaton "
      "are selected (default is " STR(DFLT_CROSS_AREA) ")" }
  , { "lifetime", OPT_LIFETIME, "N", 0,
      "Specify the automaton lifetime "
      "(default is " STR(DFLT_LIFETIME) ")" }
  , { "stat-report-rate", OPT_STAT_REPORT_RATE, "N", 0,
      "Report statistics every N steps to a file "
      "(default is " STR(DFLT_STAT_REPORT_RATE) ")" }
  , { "stat-flush-rate", OPT_STAT_FLUSH_RATE, "N", 0,
      "Flush statistics every N reports "
      "(default is " STR(DFLT_STAT_FLUSH_RATE) ")" }
  , { "example-rate", OPT_EXAMPLE_RATE, "N", 0,
      "Write example automaton every N steps "
      "(default is " STR(DFLT_EXAMPLE_RATE) ")" }
  , { "image-rate", OPT_IMAGE_RATE, "N", 0,
      "Write image every N steps "
      "(default is " STR(DFLT_IMAGE_RATE) ")" }
  , { "stat-file", OPT_STAT_FILE, "FILE", 0,
      "Report stats to FILE" }
  , { "example-name", OPT_EXAMPLE_NAME, "NAME", 0,
      "Write example automatons to NAME<n>.gv, where <n> is a step number" }
  , { "image-name", OPT_IMAGE_NAME, "NAME", 0,
      "Write images to NAME<n>.png, where <n> is a step number" }
  , { "quiet", OPT_QUIET, 0, 0,
      "Be quiet" }
  , { "species-map", OPT_SPECIES_MAP, 0, 0,
      "Show species map on images" }
  , { "no-species-map", OPT_NO_SPECIES_MAP, 0, 0,
      "Do not show species map on images (default)" }
  , { "seed", OPT_SEED, "SEED", 0,
      "Set the seed of pseudo-random number generator "
      "(default is " STR(DFLT_SEED) ")" }
  , { "deterministic", OPT_DETERMINISTIC, 0, 0,
      "Use deterministic automatons" }
  , { "mistake-aware", OPT_MISTAKE_AWARE, 0, 0,
      "Use automatons that are aware of own mistakes" }
  , { "move-aware", OPT_MOVE_AWARE, 0, 0,
      "Use automatons that are aware of own random moves" }
  , { "mistake-rate", OPT_MISTAKE_RATE, "RATE", 0,
      "Specify the rate of mistakes "
      "(default is " STR(DFLT_MISTAKE_RATE) ")" }
  , { "cross-rate", OPT_CROSS_RATE, "RATE", 0,
      "Specify the probability, that new automaton is generated via crossing "
      "instead of mutating "
      "(default is " STR(DFLT_CROSS_RATE) ")" }
  , { "state-mutation-rate", OPT_STATE_MUT_RATE, "RATE", 0,
      "Specify the probability, that whole state is replaced by a new one "
      "during mutation "
      "(default is " STR(DFLT_STATE_MUT_RATE) ")" }
  , { "action-mutation-rate", OPT_ACTION_MUT_RATE, "RATE", 0,
      "Specify the probability, that single action is replaced by a new one "
      "during mutation "
      "(default is " STR(DFLT_ACTION_MUT_RATE) ")" }
  , { "edge-mutation-rate", OPT_EDGE_MUT_RATE, "RATE", 0,
      "Specify the probability, that single edge is replaced by a new one "
      "during mutation "
      "(default is " STR(DFLT_EDGE_MUT_RATE) ")" }
  , { "show-unreachable-states", OPT_SHOW_UNREACHABLE, 0, 0,
      "Show unreachable states in examample automatons" }
  , { 0 }
  };

static void parse_size_opt(char *arg, struct argp_state *state);

static void check_arg_range(
  char *arg, int *n, int min, int max, struct argp_state *state,
  const char *name)
{
  if (parse_number(arg, n, min, max) != CHECK_OK) {
    argp_error(state, "%s must be in range between %d and %d.",
      name, min, max);
  }
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  settings_t *settings = state->input;
  switch (key) {
  case OPT_BOARD_SIZE:
    parse_size_opt(arg, state);
    break;
  case OPT_STATES:
    check_arg_range(arg, &settings->state_n, 1, MAX_STATE_N, state,
      "The number of states");
    break;
  case OPT_STEPS:
    check_arg_range(arg, &settings->step_n, 0, MAX_STEP_N, state,
      "The number of steps");
    break;
  case OPT_TURNS:
    check_arg_range(arg, &settings->turn_n, 1, MAX_TURN_N, state,
      "The number of turns");
    break;
  case OPT_PLAY_AREA:
    check_arg_range(arg, &settings->play_area, 1, MAX_AREA_SIZE, state,
      "The size of the play area");
    break;
  case OPT_KILL_AREA:
    check_arg_range(arg, &settings->kill_area, 1, MAX_AREA_SIZE, state,
      "The size of the kill area");
    break;
  case OPT_CROSS_AREA:
    check_arg_range(arg, &settings->cross_area, 1, MAX_AREA_SIZE, state,
      "The size of the cross area");
    break;
  case OPT_LIFETIME:
    check_arg_range(arg, &settings->lifetime, 1, MAX_LIFETIME, state,
      "Automaton lifetime");
    break;
  case OPT_STAT_REPORT_RATE:
    check_arg_range(arg, &settings->stat_report_rate, 1, MAX_REPORT_RATE,
      state, "The rate");
    break;
  case OPT_STAT_FLUSH_RATE:
    check_arg_range(arg, &settings->stat_flush_rate, 1, MAX_REPORT_RATE,
      state, "The rate");
    break;
  case OPT_EXAMPLE_RATE:
    check_arg_range(arg, &settings->example_rate, 1, MAX_REPORT_RATE,
      state, "The rate");
    break;
  case OPT_IMAGE_RATE:
    check_arg_range(arg, &settings->example_rate, 1, MAX_REPORT_RATE,
      state, "The rate");
    break;
  case OPT_STAT_FILE:
    settings->stat_file = arg;
    if (strcmp(arg, "-") == 0) {
      settings->flags |= F_QUIET;
    }
    break;
  case OPT_EXAMPLE_NAME:
    settings->example_name = arg;
    break;
  case OPT_IMAGE_NAME:
    settings->image_name = arg;
    break;
  case OPT_QUIET:
    settings->flags |= F_QUIET;
    break;
  case OPT_SPECIES_MAP:
    settings->flags |= F_SPECIES_MAP;
    break;
  case OPT_NO_SPECIES_MAP:
    settings->flags &= ~F_SPECIES_MAP;
    break;
  case OPT_SEED:
    settings->seed = atol(arg);
    break;
  case OPT_DETERMINISTIC:
    settings->flags |= F_DETERMINISTIC;
    break;
  case OPT_MISTAKE_AWARE:
    settings->flags |= F_MISTAKE_AWARE;
    break;
  case OPT_MOVE_AWARE:
    settings->flags |= F_MOVE_AWARE;
    break;
  case OPT_MISTAKE_RATE:
    settings->mistake_rate = atof(arg);
    break;
  case OPT_CROSS_RATE:
    settings->cross_rate = atof(arg);
    break;
  case OPT_STATE_MUT_RATE:
    settings->state_mut_rate = atof(arg);
    break;
  case OPT_ACTION_MUT_RATE:
    settings->action_mut_rate = atof(arg);
    break;
  case OPT_EDGE_MUT_RATE:
    settings->edge_mut_rate = atof(arg);
    break;
  case OPT_SHOW_UNREACHABLE:
    settings->flags |= F_SHOW_UNREACHABLE;
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
    argp_error(state, "Board size must be in range between 1 and "
      STR(MAX_BOARD_SIZE) ".");
    break;
  case PARSE_SIZE_TOO_SMALL:
    argp_error(state, "Board is too small.");
    break;
  default:
    assert(0);
  }
}

static struct argp argp = { options, parse_opt, 0, doc, 0, 0, 0 };

int main(int argc, char **argv) {
  world_t world =
    { .settings = 
      { .board_size_x       = DFLT_BOARD_SIZE
      , .board_size_y       = DFLT_BOARD_SIZE
      , .state_n            = DFLT_STATES
      , .step_n             = DFLT_STEPS
      , .turn_n             = DFLT_TURNS
      , .play_area          = DFLT_PLAY_AREA
      , .kill_area          = DFLT_KILL_AREA
      , .cross_area         = DFLT_CROSS_AREA
      , .lifetime           = DFLT_LIFETIME
      , .stat_report_rate   = DFLT_STAT_REPORT_RATE
      , .stat_flush_rate    = DFLT_STAT_FLUSH_RATE
      , .example_rate       = DFLT_EXAMPLE_RATE
      , .image_rate         = DFLT_IMAGE_RATE
      , .flags              = 0
      , .seed               = DFLT_SEED
      , .mistake_rate       = DFLT_MISTAKE_RATE
      , .cross_rate         = DFLT_CROSS_RATE
      , .state_mut_rate     = DFLT_STATE_MUT_RATE
      , .action_mut_rate    = DFLT_ACTION_MUT_RATE
      , .edge_mut_rate      = DFLT_EFGE_MUT_RATE
      , .stat_file          = DFLT_STAT_FILE
      , .example_name       = DFLT_EXAMPLE_NAME
      , .image_name         = DFLT_IMAGE_NAME
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
  } while (world_next_step(&world));

  world_destroy(&world);
  return 0;
}
