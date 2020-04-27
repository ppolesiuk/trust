#include "automaton.h"

#include <assert.h>

#define ACTION_RESOLUTION 1024

static void state_init(state_t *st, unsigned short state_n, MTRand *rand) {
  st->action = genRandLong(rand)%(ACTION_RESOLUTION + 1);
  for (int i = 0; i < 8; ++i) {
    st->next_tab[i] = genRandLong(rand)%state_n;
  }
}

void automaton_init(automaton_t *a, const settings_t *settings, MTRand *rand) {
  a->score    = 0;
  a->state_n  = settings->state_n;
  a->lifetime = settings->lifetime;
  a->status   = A_ST_ALIVE;
  a->color    = genRandLong(rand) & 0xFFFFFF;
  a->states   = malloc(sizeof(state_t) * a->state_n);

  for (int i = 0; i < (int)a->state_n; ++i) {
    state_init(&a->states[i], a->state_n, rand);
  }
}

void automaton_destroy(automaton_t *a) {
  free(a->states);
}

void automaton_reset(automaton_t *a) {
  a->score  = 0;
  a->status = A_ST_ALIVE;
  if (a->lifetime > 0) {
    a->lifetime--;
  }
}

void automaton_play(
  automaton_t       *a1,
  automaton_t       *a2,
  const settings_t  *settings,
  MTRand            *rand)
{
  int s1 = 0;
  int s2 = 0;
  for (int i = 0; i < settings->turn_n; i++) {
    int err1 = (genRand(rand) < 0.01 ? 1 : 0);
    int err2 = (genRand(rand) < 0.01 ? 1 : 0);
    int act1 =
      (genRandLong(rand)%ACTION_RESOLUTION < a1->states[s1].action ? 1 : 0);
    int act2 =
      (genRandLong(rand)%ACTION_RESOLUTION < a2->states[s2].action ? 1 : 0);
    act1 = (err1 ? 1-act1 : act1);
    act2 = (err2 ? 1-act2 : act2);
    a1->score += 3*act2 - act1;
    a2->score += 3*act1 - act2;
    s1 = a1->states[s1].next[err1][act1][act2];
    s2 = a2->states[s2].next[err2][act2][act1];
  }
}

static unsigned cross_color(unsigned c1, unsigned c2, MTRand *rand) {
  int      x = genRandLong(rand) % 54;
  unsigned c = (x & 1 ? c1 : c2);
  int      r = (c & 0xFF)         + (x / 2) % 3 - 1;
  r = (r < 0 ? 0 : r > 255 ? 255 : r);
  int      g = ((c >> 8)  & 0xFF) + (x / 6) % 3 - 1;
  g = (g < 0 ? 0 : g > 255 ? 255 : g);
  int      b = ((c >> 16) & 0xFF) + (x / 18) - 1;
  b = (b < 0 ? 0 : b > 255 ? 255 : b);
  return (b << 16) | (g << 8) | r;
}

void automaton_cross(
  automaton_t       *a,
  const automaton_t *p1,
  const automaton_t *p2,
  MTRand            *rand)
{
  int i;
  assert(a->state_n == p1->state_n && a->state_n == p2->state_n);
  a->color = cross_color(p1->color, p2->color, rand);
  for (i = 0; i < (int)a->state_n; ++i) {
    if (genRand(rand) < 0.01) {
      state_init(&a->states[i], a->state_n, rand);
      continue;
    } else if (genRandLong(rand) % 2 == 0) {
      a->states[i] = p1->states[i];
    } else {
      a->states[i] = p2->states[i];
    }
    if (genRand(rand) < 0.01) {
      a->states[i].action = genRandLong(rand)%(ACTION_RESOLUTION + 1);
    }
    for (int j = 0; j < 8; ++j) {
      if (genRand(rand) < 0.01) {
        a->states[i].next_tab[j] = genRandLong(rand) % a->state_n;
      }
    }
  }
}

void automaton_print(FILE *file, automaton_t *a) {
  int i;
  fprintf(file, "digraph automaton {\n");
  fprintf(file, "  node [shape = doublecircle, label = \"S%0.3f\"] ST_0;\n",
    (float)a->states[0].action / ACTION_RESOLUTION);
  for (i = 1; i < a->state_n; ++i) {
    fprintf(file, "  node [shape = circle, label = \"%0.3f\"] ST_%d;\n",
      (float)a->states[i].action / ACTION_RESOLUTION,
      i);
  }
  for (i = 0; i < a->state_n; ++i) {
    if (a->states[i].action != 0) {
      fprintf(file, "  ST_%d -> ST_%d [label = \"_10\"];\n",
        i, (int)a->states[i].next[0][1][0]);
      fprintf(file, "  ST_%d -> ST_%d [label = \"_11\"];\n",
        i, (int)a->states[i].next[0][1][1]);
      fprintf(file, "  ST_%d -> ST_%d [label = \"!00\"];\n",
        i, (int)a->states[i].next[1][0][0]);
      fprintf(file, "  ST_%d -> ST_%d [label = \"!01\"];\n",
        i, (int)a->states[i].next[1][0][1]);
    }
    if (a->states[i].action != ACTION_RESOLUTION) {
      fprintf(file, "  ST_%d -> ST_%d [label = \"_00\"];\n",
        i, (int)a->states[i].next[0][0][0]);
      fprintf(file, "  ST_%d -> ST_%d [label = \"_01\"];\n",
        i, (int)a->states[i].next[0][0][1]);
      fprintf(file, "  ST_%d -> ST_%d [label = \"!10\"];\n",
        i, (int)a->states[i].next[1][1][0]);
      fprintf(file, "  ST_%d -> ST_%d [label = \"!11\"];\n",
        i, (int)a->states[i].next[1][1][1]);
    }
  }
  fprintf(file, "}\n");
}
