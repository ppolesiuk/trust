#include "automaton.h"

#include <stdio.h>

#define BOARD_SIZE_X 53
#define BOARD_SIZE_Y 69

#define PLAY_AREA    3
#define KILL_AREA    2
#define NEW_AREA     4

#define TURN_N       50
#define STATE_N      12
#define STEP_N       3000

static automaton_t world[BOARD_SIZE_X * BOARD_SIZE_Y];

/* ========================================================================= */

static void world_init(void) {
  for (int i = 0; i < BOARD_SIZE_X * BOARD_SIZE_Y; ++i) {
    automaton_init(&world[i], STATE_N);
  }
}

/* ========================================================================= */

static void world_reset(void) {
  for (int i = 0; i < BOARD_SIZE_X * BOARD_SIZE_Y; ++i) {
    automaton_reset(&world[i]);
  }
}

/* ========================================================================= */

static void world_play_with(int x, int y) {
  int i = y * BOARD_SIZE_X + x;
  for (int dy = -PLAY_AREA; dy <= PLAY_AREA; ++dy) {
    for (int dx = -PLAY_AREA; dx <= PLAY_AREA; ++dx) {
      int x2 = (x + BOARD_SIZE_X + dx) % BOARD_SIZE_X;
      int y2 = (y + BOARD_SIZE_Y + dy) % BOARD_SIZE_Y;
      int j = y2 * BOARD_SIZE_X + x2;
      if (i != j) {
        automaton_play(&world[i], &world[j], TURN_N);
      }
    }
  }
}

static void world_play(void) {
  for (int y = 0; y < BOARD_SIZE_Y; ++y) {
    for (int x = 0; x < BOARD_SIZE_X; ++x) {
      world_play_with(x, y);
    }
  }
}

/* ========================================================================= */

static void world_kill_if_weak(int x, int y) {
  int i = y * BOARD_SIZE_X + x;
  if (world[i].status != A_ST_ALIVE) {
    return;
  }
  if (rand() % 500 != 0) {
    for (int dy = -KILL_AREA; dy <= KILL_AREA; ++dy) {
      for (int dx = -KILL_AREA; dx <= KILL_AREA; ++dx) {
        int x2 = (x + BOARD_SIZE_X + dx) % BOARD_SIZE_X;
        int y2 = (y + BOARD_SIZE_Y + dy) % BOARD_SIZE_Y;
        int j = y2 * BOARD_SIZE_X + x2;
        if (world[i].score > world[j].score) {
          world[i].status = A_ST_SURVIVED;
          return;
        }
      }
    }
  }
  for (int dy = -KILL_AREA; dy <= KILL_AREA; ++dy) {
    for (int dx = -KILL_AREA; dx <= KILL_AREA; ++dx) {
      int x2 = (x + BOARD_SIZE_X + dx) % BOARD_SIZE_X;
      int y2 = (y + BOARD_SIZE_Y + dy) % BOARD_SIZE_Y;
      int j = y2 * BOARD_SIZE_X + x2;
      world[j].status = A_ST_SURVIVED;
    }
  }
  world[i].status = A_ST_DEAD;
}

static void world_kill_weak(void) {
  for (int y = 0; y < BOARD_SIZE_Y; ++y) {
    for (int x = 0; x < BOARD_SIZE_X; ++x) {
      world_kill_if_weak(x, y);
    }
  }
}

/* ========================================================================= */

static FILE *history;
static int step;

static void world_display(void) {
  long total = 0;
  for (int i = 0; i < BOARD_SIZE_X * BOARD_SIZE_Y; ++i) {
    total += world[i].score;
  }
  float avg = (float)total / (BOARD_SIZE_X * BOARD_SIZE_Y);
  fprintf(history, "%d\t%f\n", step, avg);
  fflush(history);
  if (step % 10 != 0) { return; };
  for (int y = 0; y < BOARD_SIZE_Y; ++y) {
    for (int x = 0; x < BOARD_SIZE_X; ++x) {
      int i = y * BOARD_SIZE_X + x;
      if (x != 0) {
        fprintf(stderr, "|");
      }
      fprintf(stderr, "%s%4d%s",
        (world[i].status == A_ST_DEAD ? "\033[31m" : ""),
        world[i].score,
        (world[i].status == A_ST_DEAD ? "\033[0m" : ""));
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "AVG[%d]: %f\n", step, avg);
}

/* ========================================================================= */

static int select_parent(int x, int y) {
  int dx = rand() % (2*NEW_AREA + 1) - NEW_AREA;
  int dy = rand() % (2*NEW_AREA + 1) - NEW_AREA;
  int x2 = (x + BOARD_SIZE_X + dx) % BOARD_SIZE_X;
  int y2 = (y + BOARD_SIZE_Y + dy) % BOARD_SIZE_Y;
  int j = y2 * BOARD_SIZE_X + x2;
  return (world[j].status == A_ST_SURVIVED) ? j : -1;
}

static void world_create_new(void) {
  for (int y = 0; y < BOARD_SIZE_Y; ++y) {
    for (int x = 0; x < BOARD_SIZE_X; ++x) {
      int i = y * BOARD_SIZE_X + x;
      if (world[i].status != A_ST_DEAD) {
        continue;
      }
      int j, k;
      do { j = select_parent(x, y); } while (j == -1);
      do { k = select_parent(x, y); } while (k == -1 && j != k);
      automaton_cross(&world[i], &world[j], &world[k]);
    }
  }
}

/* ========================================================================= */

#define BUFLEN 4096
void skip_line(void) {
/*
  static char buf[BUFLEN];
  fgets(buf, BUFLEN, stdin);
*/
  fflush(stderr);
}

/* ========================================================================= */

int main(int argc, char **argv) {
  world_init();
  history = fopen("history", "wt");
  for (step = 0; step < STEP_N; ++step) {
    world_reset();
    world_play();
    world_kill_weak();
    world_display();
    skip_line();
    world_create_new();
  }
  fclose(history);
  int i;
  do {
    i = rand()%(BOARD_SIZE_X * BOARD_SIZE_Y);
  } while (world[i].status != A_ST_SURVIVED);
  automaton_print(stdout, &world[i]);
  return 0;
}
