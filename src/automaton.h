#ifndef __AUTOMATON_H
#define __AUTOMATON_H

#include "settings.h"
#include "mtwister.h"

#include <stdlib.h>
#include <stdio.h>

#define A_ST_ALIVE    0
#define A_ST_STRONG   1
#define A_ST_SURVIVED 2
#define A_ST_DEAD     3

typedef struct state {
  unsigned short action;
  union {
    unsigned short next_tab[8];
    unsigned short next[2][2][2];
  };
} state_t;

typedef struct automaton {
  int            score;
  unsigned short state_n;
  unsigned short lifetime;
  char           status;
  unsigned       color;
  state_t       *states;
} automaton_t;

void automaton_init(automaton_t *a, const settings_t *settings, MTRand *rand);
void automaton_destroy(automaton_t *a);

void automaton_reset(automaton_t *a);

void automaton_play(
  automaton_t      *a1,
  automaton_t      *a2,
  const settings_t *settings,
  MTRand           *rand);

void automaton_cross(
  automaton_t       *a,
  const automaton_t *p1,
  const automaton_t *p2,
  MTRand            *rand);

void automaton_print(FILE *file, automaton_t *a);

#endif
