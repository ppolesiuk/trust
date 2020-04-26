#ifndef __AUTOMATON_H__
#define __AUTOMATON_H__

#include "settings.h"

#include <stdlib.h>
#include <stdio.h>

#define A_ST_ALIVE    0
#define A_ST_SURVIVED 1
#define A_ST_DEAD     2

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
  char           status;
  unsigned char  color[3];
  state_t       *states;
} automaton_t;

void automaton_init(automaton_t *a, unsigned short state_n);
void automaton_destroy(automaton_t *a);

void automaton_reset(automaton_t *a);

void automaton_play(automaton_t *a1, automaton_t *a2, settings_t *settings);

void automaton_cross(
  automaton_t       *a,
  const automaton_t *p1,
  const automaton_t *p2);

void automaton_print(FILE *file, automaton_t *a);

#endif
