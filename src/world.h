#ifndef __WORLD_H
#define __WORLD_H

#include "automaton.h"
#include "settings.h"
#include "mtwister.h"

#include <stdio.h>

typedef struct world {
  settings_t    settings;
  unsigned long step;
  automaton_t  *pop;
  FILE         *stat_file;
  MTRand        rand;
} world_t;

void world_init(world_t *world);
void world_destroy(world_t *world);

void world_reset(world_t *world);
void world_play(world_t *world);
void world_kill_weak(world_t *world);
void world_spawn_new(world_t *world);
void world_report(world_t *world);

int world_next_step(world_t *world);

void world_serialize(const world_t *world);
void world_deserialize(world_t *world);

#endif
