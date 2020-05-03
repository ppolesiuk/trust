#include "world.h"
#include "world_image.h"
#include "serialization.h"

#include <errno.h>
#include <error.h>
#include <limits.h>
#include <string.h>

static int board_size(const world_t *world) {
  return world->settings.board_size_x * world->settings.board_size_y;
}

static void world_basic_init(world_t *world, int continued) {
  world->pop = malloc(sizeof(automaton_t) * board_size(world));
  if (world->settings.stat_file == NULL) {
    world->stat_file = NULL;
  } else if (strcmp(world->settings.stat_file, "-") == 0) {
    world->stat_file = stdout;
  } else {
    world->stat_file =
      fopen(world->settings.stat_file, (continued ? "at" : "wt"));
    if (world->stat_file == NULL) {
      error(EXIT_FAILURE, errno, "cannot open file `%s'",
        world->settings.stat_file);
    }
  }
}

void world_init(world_t *world) {
  world_basic_init(world, 0);
  world->rand = seedRand(world->settings.seed);
  world->step = 0;
  for (int i = 0; i < board_size(world); ++i) {
    automaton_init(&world->pop[i], &world->settings, &world->rand);
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
        automaton_play(&world->pop[i], &world->pop[j],
          &world->settings, &world->rand);
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
  for (int dy = -kill_area; dy <= kill_area; ++dy) {
    for (int dx = -kill_area; dx <= kill_area; ++dx) {
      int x2 = mod(x + dx, size_x);
      int y2 = mod(y + dy, size_y);
      int j = y2 * size_x + x2;
      if (world->pop[i].score > world->pop[j].score) {
        world->pop[i].status = A_ST_STRONG;
        return;
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

static void world_kill_if_old(world_t *world, int x, int y) {
  int size_x = world->settings.board_size_x;
  int size_y = world->settings.board_size_y;
  int i = y * size_x + x;
  int kill_area = world->settings.kill_area;
  if (world->pop[i].status != A_ST_STRONG ||
    world->pop[i].lifetime != 0)
  {
    return;
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
  for (int y = 0; y < world->settings.board_size_y; ++y) {
    for (int x = 0; x < world->settings.board_size_x; ++x) {
      world_kill_if_old(world, x, y);
    }
  }
}

static int select_parent(world_t *world, int x, int y) {
  int size_x = world->settings.board_size_x;
  int size_y = world->settings.board_size_y;
  int cross_area = world->settings.cross_area;
  int dx = genRandLong(&world->rand) % (2*cross_area + 1) - cross_area;
  int dy = genRandLong(&world->rand) % (2*cross_area + 1) - cross_area;
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
      automaton_cross(&world->pop[i], &world->pop[j], &world->pop[k],
        &world->settings, &world->rand);
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
    /* we use different PRNG, in order to make simulation deterministic */
    i = rand() % board_size(world);
  } while (world->pop[i].status != A_ST_SURVIVED);
  return &world->pop[i];
}

static void report_example_automaton(world_t *world) {
  char *buf = malloc(strlen(world->settings.example_name) + 32);
  sprintf(buf, "%s%lu.gv", world->settings.example_name, world->step);
  FILE *file = fopen(buf, "w");
  if (file) {
    automaton_print(file, &world->settings, pick_example_automaton(world));
    fclose(file);
  } else {
    error(0, errno, "cannot open file `%s'", buf);
  }
  free(buf);
}

static void report_image(world_t *world) {
  char title[64];
  char *fname = malloc(strlen(world->settings.image_name) + 32);
  sprintf(fname, "%s%lu.png", world->settings.image_name, world->step);
  sprintf(title, "Step %ld", world->step);
  write_world_image(fname, world, title);
  free(fname);
}

void world_report(world_t *world) {
  if (world->stat_file) {
    if (world->step % world->settings.stat_report_rate == 0) {
      fprintf(world->stat_file, "%lu\t%f\n", world->step, avg_score(world));
    }
    unsigned long rs = world->step / world->settings.stat_report_rate;
    if (rs % world->settings.stat_flush_rate == 0) {
      fflush(world->stat_file);
    }
  }
  if (world->settings.example_name != NULL
    && world->step % world->settings.example_rate == 0)
  {
    report_example_automaton(world);
  }
  if (world->settings.image_name != NULL
    && world->step % world->settings.image_rate == 0)
  {
    report_image(world);
  }
  if ((world->settings.flags & F_QUIET) == 0) {
    printf("\r%10lu: %10f", world->step, avg_score(world));
    fflush(stdout);
  }
}

int world_next_step(world_t *world) {
  world->step++;
  return world->settings.step_n == 0
      || world->step < world->settings.step_n;
}

#define TMP_WORLD_FILE ".world_new"
#define WORLD_FILE "world"

static void world_serialize_main(FILE *file, const world_t *world) {
  serialize_tag(file, "WORLD");
  SERIALIZE_ULONG(file, world, step);
  for (int i = 0; i < board_size(world); ++i) {
    automaton_serialize(file, &world->pop[i]);
  }
} 

static void world_deserialize_main(FILE *file, world_t *world) {
  deserialize_tag(file, "WORLD");
  DESERIALIZE_ULONG(file, world, step, 0, ULONG_MAX);
  for (int i = 0; i < board_size(world); ++i) {
    automaton_deserialize(file, &world->pop[i]);
  }
}

void world_serialize(const world_t *world) {
  FILE *file = fopen(TMP_WORLD_FILE, "w");
  if (file == NULL) {
    error(0, errno, "cannot open world file `%s'", TMP_WORLD_FILE);
    return;
  }

  serialize_version(file, "trust_version", TRUST_VERSION);
  settings_serialize(file, &world->settings);
  world_serialize_main(file, world);
  serializeRand(file, &world->rand);

  fclose(file);
  if (rename(TMP_WORLD_FILE, WORLD_FILE)) {
    error(0, errno, "cannot move world file to `%s'", WORLD_FILE);
  }
}

void world_deserialize(world_t *world) {
  FILE *file = fopen(WORLD_FILE, "r");
  if (file == NULL) {
    error(EXIT_FAILURE, errno, "cannot open world file `%s'", TMP_WORLD_FILE);
    return;
  }

  deserialize_version(file, "trust_version", TRUST_VERSION);
  settings_deserialize(file, &world->settings);
  world_basic_init(world, 1);
  world_deserialize_main(file, world);
  deserializeRand(file, &world->rand);

  fclose(file);
}
