#include "world.h"

#include <errno.h>
#include <error.h>
#include <string.h>

static int board_size(world_t *world) {
  return world->settings.board_size_x * world->settings.board_size_y;
}

void world_init(world_t *world) {
  world->pop  = malloc(sizeof(automaton_t) * board_size(world));
  world->step = 0;
  for (int i = 0; i < board_size(world); ++i) {
    automaton_init(&world->pop[i], world->settings.state_n);
  }
  if (strcmp(world->settings.stat_file, "") == 0) {
    world->stat_file = NULL;
  } else if (strcmp(world->settings.stat_file, "-") == 0) {
    world->stat_file = stdout;
  } else {
    world->stat_file = fopen(world->settings.stat_file, "wt");
    if (world->stat_file == NULL) {
      error(EXIT_FAILURE, errno, "cannot open file `%s'",
        world->settings.stat_file);
    }
  }
}

void world_destroy(world_t *world) {
  for (int i = 0; i < board_size(world); ++i) {
    automaton_destroy(&world->pop[i]);
  }
  free(world->pop);
  if (world->stat_file != NULL && world->stat_file != stdout) {
    fclose(world->stat_file);
  }
}

void world_reset(world_t *world) {
  for (int i = 0; i < board_size(world); ++i) {
    automaton_reset(&world->pop[i]);
  }
}

static int mod(int x, int y) {
  x %= y;
  return x < 0 ? x + y : x;
}

static void world_play_with(world_t *world, int x, int y) {
  int size_x = world->settings.board_size_x;
  int size_y = world->settings.board_size_y;
  int i = y * size_x + x;
  int play_area = world->settings.play_area;
  for (int dy = -play_area; dy <= play_area; ++dy) {
    for (int dx = -play_area; dx <= play_area; ++dx) {
      int x2 = mod(x + dx, size_x);
      int y2 = mod(y + dy, size_y);
      int j = y2 * size_x + x2;
      if (i != j) {
        automaton_play(&world->pop[i], &world->pop[j], &world->settings);
      }
    }
  }
}

void world_play(world_t *world) {
  for (int y = 0; y < world->settings.board_size_y; ++y) {
    for (int x = 0; x < world->settings.board_size_x; ++x) {
      world_play_with(world, x, y);
    }
  }
}

static void world_kill_if_weak(world_t *world, int x, int y) {
  int size_x = world->settings.board_size_x;
  int size_y = world->settings.board_size_y;
  int i = y * size_x + x;
  int kill_area = world->settings.kill_area;
  if (world->pop[i].status != A_ST_ALIVE) {
    return;
  }
  if (rand() % 500 != 0) {
    for (int dy = -kill_area; dy <= kill_area; ++dy) {
      for (int dx = -kill_area; dx <= kill_area; ++dx) {
        int x2 = mod(x + dx, size_x);
        int y2 = mod(y + dy, size_y);
        int j = y2 * size_x + x2;
        if (world->pop[i].score > world->pop[j].score) {
          world->pop[i].status = A_ST_SURVIVED;
          return;
        }
      }
    }
  }
  for (int dy = -kill_area; dy <= kill_area; ++dy) {
    for (int dx = -kill_area; dx <= kill_area; ++dx) {
      int x2 = mod(x + dx, size_x);
      int y2 = mod(y + dy, size_y);
      int j = y2 * size_x + x2;
      world->pop[j].status = A_ST_SURVIVED;
    }
  }
  world->pop[i].status = A_ST_DEAD;
}

void world_kill_weak(world_t *world) {
  for (int y = 0; y < world->settings.board_size_y; ++y) {
    for (int x = 0; x < world->settings.board_size_x; ++x) {
      world_kill_if_weak(world, x, y);
    }
  }
}

static int select_parent(world_t *world, int x, int y) {
  int size_x = world->settings.board_size_x;
  int size_y = world->settings.board_size_y;
  int cross_area = world->settings.cross_area;
  int dx = rand() % (2*cross_area + 1) - cross_area;
  int dy = rand() % (2*cross_area + 1) - cross_area;
  int x2 = mod(x + dx, size_x);
  int y2 = mod(y + dy, size_y);
  int j = y2 * size_x + x2;
  return (world->pop[j].status == A_ST_SURVIVED) ? j : -1;
}

void world_spawn_new(world_t *world) {
  for (int y = 0; y < world->settings.board_size_y; ++y) {
    for (int x = 0; x < world->settings.board_size_x; ++x) {
      int i = y * world->settings.board_size_x + x;
      if (world->pop[i].status != A_ST_DEAD) {
        continue;
      }
      int j, k;
      do { j = select_parent(world, x, y); } while (j == -1);
      do { k = select_parent(world, x, y); } while (k == -1 && j != k);
      automaton_cross(&world->pop[i], &world->pop[j], &world->pop[k]);
    }
  }
}

static double avg_score(world_t *world) {
  long sum = 0;
  for (int i = 0; i < board_size(world); ++i) {
    sum += world->pop[i].score;
  }
  return (double)sum / board_size(world);
}

static automaton_t *pick_example_automaton(world_t *world) {
  int i;
  do {
    i = rand() % board_size(world);
  } while (world->pop[i].status != A_ST_SURVIVED);
  return &world->pop[i];
}

static void report_example_automaton(world_t *world) {
  char *buf = malloc(strlen(world->settings.automaton_file) + 32);
  sprintf(buf, "%s%lu.gv", world->settings.automaton_file, world->step);
  FILE *file = fopen(buf, "w");
  if (file) {
    automaton_print(file, pick_example_automaton(world));
    fclose(file);
  }
  free(buf);
}

void world_report(world_t *world) {
  if (world->stat_file) {
    if (world->step % world->settings.stat_report_step == 0) {
      fprintf(world->stat_file, "%lu\t%f\n", world->step, avg_score(world));
    }
    if (world->step % world->settings.stat_flush_step == 0) {
      fflush(world->stat_file);
    }
  }
  if (world->settings.aexample_step != 0
    && world->step % world->settings.aexample_step == 0)
  {
    report_example_automaton(world);
  }
}

int world_next_step(world_t *world) {
  world->step++;
  return world->settings.step_n == 0
      || world->step < world->settings.step_n;
}