#include "automaton.h"

#include <assert.h>

#define ACTION_RESOLUTION 1024

void state_init(state_t *st, unsigned short state_n) {
  st->action = rand()%(ACTION_RESOLUTION + 1);
  for (int i = 0; i < 8; ++i) {
    st->next_tab[i] = rand()%state_n;
  }
}

void automaton_init(automaton_t *a, unsigned short state_n) {
  a->score   = 0;
  a->state_n = state_n;
  a->status  = A_ST_ALIVE;
  a->color[0] = rand()%256;
  a->color[1] = rand()%256;
  a->color[2] = rand()%256;
  a->states  = malloc(sizeof(state_t) * state_n);

  for (int i = 0; i < (int)state_n; ++i) {
    state_init(&a->states[i], state_n);
  }
}

void automaton_destroy(automaton_t *a) {
  free(a->states);
}

void automaton_reset(automaton_t *a) {
  a->score  = 0;
  a->status = A_ST_ALIVE;
}

void automaton_play(automaton_t *a1, automaton_t *a2, settings_t *settings) {
  int s1 = 0;
  int s2 = 0;
  for (int i = 0; i < settings->turn_n; i++) {
    int err1 = (rand()%100 < 1 ? 1 : 0);
    int err2 = (rand()%100 < 1 ? 1 : 0);
    int act1 = (rand()%ACTION_RESOLUTION < a1->states[s1].action ? 1 : 0);
    int act2 = (rand()%ACTION_RESOLUTION < a2->states[s2].action ? 1 : 0);
    act1 = (err1 ? 1-act1 : act1);
    act2 = (err2 ? 1-act2 : act2);
    a1->score += 3*act2 - act1;
    a2->score += 3*act1 - act2;
    s1 = a1->states[s1].next[err1][act1][act2];
    s2 = a2->states[s2].next[err2][act2][act1];
  }
}

static unsigned char change_color(int c) {
  c += rand()%3 - 1;
  return (c < 0 ? 0 : c > 255 ? 255 : c);
}

void automaton_cross(
  automaton_t       *a,
  const automaton_t *p1,
  const automaton_t *p2)
{
  int i;
  assert(a->state_n == p1->state_n && a->state_n == p2->state_n);
  for (i = 0; i < 3; ++i) {
    a->color[i] = change_color((rand()%2) ? p1->color[i] : p2->color[i]);
  }
  for (i = 0; i < (int)a->state_n; ++i) {
    if (rand() % 100 == 0) {
      state_init(&a->states[i], a->state_n);
      continue;
    } else if (rand() % 2 == 0) {
      a->states[i] = p1->states[i];
    } else {
      a->states[i] = p2->states[i];
    }
    if (rand() % 100 == 0) {
      a->states[i].action = rand()%(ACTION_RESOLUTION + 1);
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
