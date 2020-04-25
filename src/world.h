#ifndef __WORLD_H__
#define __WORLD_H__

#include "automaton.h"
#include "settings.h"

#include <stdio.h>

typedef struct world {
  settings_t    settings;
  unsigned long step;
  automaton_t  *pop;
  FILE         *stat_file;
} world_t;

void world_init(world_t *world);
void world_destroy(world_t *world);

void world_reset(world_t *world);
void world_play(world_t *world);
void world_kill_weak(world_t *world);
void world_spawn_new(world_t *world);
void world_report(world_t *world);

int world_next_step(world_t *world);

#endif