#include "automaton.h"

#include <assert.h>
#include <string.h>

#define ACTION_RESOLUTION 1024

static unsigned short rand_action(const settings_t *settings, MTRand *rand) {
  if ((settings->flags & F_DETERMINISTIC) == 0) {
    return genRandLong(rand)%(ACTION_RESOLUTION + 1);
  } else {
    return (genRandLong(rand) & 1) * ACTION_RESOLUTION;
  }
}

static void state_init(state_t *st, const settings_t *settings, MTRand *rand) {
  st->action = rand_action(settings, rand);
  for (int i = 0; i < 8; ++i) {
    st->next_tab[i] = genRandLong(rand)%settings->state_n;
  }
}

void automaton_init(automaton_t *a, const settings_t *settings, MTRand *rand) {
  a->score    = 0;
  a->state_n  = settings->state_n;
  a->lifetime = genRandLong(rand) % settings->lifetime;
  a->status   = A_ST_ALIVE;
  a->color    = genRandLong(rand) & 0xFFFFFF;
  a->states   = malloc(sizeof(state_t) * a->state_n);

  for (int i = 0; i < (int)a->state_n; ++i) {
    state_init(&a->states[i], settings, rand);
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
    int err1 = (genRandFixed(rand) < settings->mistake_rate ? 1 : 0);
    int err2 = (genRandFixed(rand) < settings->mistake_rate ? 1 : 0);
    int act1 =
      (genRandLong(rand)%ACTION_RESOLUTION < a1->states[s1].action ? 1 : 0);
    int act2 =
      (genRandLong(rand)%ACTION_RESOLUTION < a2->states[s2].action ? 1 : 0);
    act1 = (err1 ? 1-act1 : act1);
    act2 = (err2 ? 1-act2 : act2);
    a1->score += 3*act2 - act1;
    a2->score += 3*act1 - act2;
    int own1 = act1;
    int own2 = act2;
    if ((settings->flags & F_MISTAKE_AWARE) == 0) {
      err1 = 0;
      err2 = 0;
    }
    if ((settings->flags & F_MOVE_AWARE) == 0) {
      own1 = 0;
      own2 = 0;
    }
    s1 = a1->states[s1].next[err1][own1][act2];
    s2 = a2->states[s2].next[err2][own2][act1];
  }
}

static unsigned mutate_color(unsigned c, MTRand *rand) {
  int x = genRandLong(rand) % 27;
  int r = (c & 0xFF) + x % 3 - 1;
  int g = ((c >> 8)  & 0xFF) + (x / 3) % 3 - 1;
  int b = ((c >> 16) & 0xFF) + (x / 9) - 1;
  r = (r < 0 ? 0 : r > 255 ? 255 : r);
  g = (g < 0 ? 0 : g > 255 ? 255 : g);
  b = (b < 0 ? 0 : b > 255 ? 255 : b);
  return (b << 16) | (g << 8) | r;
}

void automaton_cross(
  automaton_t       *a,
  const automaton_t *p1,
  const automaton_t *p2,
  const settings_t  *settings,
  MTRand            *rand)
{
  int i;
  assert(a->state_n == p1->state_n && a->state_n == p2->state_n);
  a->lifetime = genRandLong(rand) % settings->lifetime;
  if (genRandFixed(rand) < settings->cross_rate) {
    a->color = mutate_color(
      (genRandLong(rand) % 2 == 0 ? p1->color : p2->color),
      rand);
    for (i = 0; i < (int)a->state_n; ++i) {
      a->states[i] =
        (genRandLong(rand) % 2 == 0 ? p1->states[i] : p2->states[i]);
    }
  } else {
    a->color = mutate_color(p1->color, rand);
    for (i = 0; i < (int)a->state_n; ++i) {
      a->states[i] = p1->states[i];
    }
  }
  for (i = 0; i < (int)a->state_n; ++i) {
    if (genRandFixed(rand) < settings->state_mut_rate) {
      state_init(&a->states[i], settings, rand);
      continue;
    }
    if (genRandFixed(rand) < settings->action_mut_rate) {
      a->states[i].action = rand_action(settings, rand);
    }
    for (int j = 0; j < 8; ++j) {
      if (genRandFixed(rand) < settings->edge_mut_rate) {
        a->states[i].next_tab[j] = genRandLong(rand) % a->state_n;
      }
    }
  }
}

static void find_reachable_states(
  const automaton_t *a,
  const settings_t  *settings,
  unsigned short *reachable)
{
  int st = 0;
  int next;
  reachable[st] = 1;
  while (1) {
    if ((settings->flags & F_MOVE_AWARE) == 0) {
      next = a->states[st].next[0][0][0];
      if (reachable[next] == 0) goto go_down;
      next = a->states[st].next[0][0][1];
      if (reachable[next] == 0) goto go_down;
      if ((settings->flags & F_MISTAKE_AWARE)
        && settings->mistake_rate > 0.0)
      {
        next = a->states[st].next[1][0][0];
        if (reachable[next] == 0) goto go_down;
        next = a->states[st].next[1][0][1];
        if (reachable[next] == 0) goto go_down;
      }
    } else {
      if (a->states[st].action != 0) {
        next = a->states[st].next[0][1][0];
        if (reachable[next] == 0) goto go_down;
        next = a->states[st].next[0][1][1];
        if (reachable[next] == 0) goto go_down;
        if ((settings->flags & F_MISTAKE_AWARE)
          && settings->mistake_rate > 0.0)
        {
          next = a->states[st].next[1][0][0];
          if (reachable[next] == 0) goto go_down;
          next = a->states[st].next[1][0][1];
          if (reachable[next] == 0) goto go_down;
        }
      }
      if (a->states[st].action != ACTION_RESOLUTION) {
        next = a->states[st].next[0][0][0];
        if (reachable[next] == 0) goto go_down;
        next = a->states[st].next[0][0][1];
        if (reachable[next] == 0) goto go_down;
        if ((settings->flags & F_MISTAKE_AWARE)
          && settings->mistake_rate > 0.0)
        {
          next = a->states[st].next[1][1][0];
          if (reachable[next] == 0) goto go_down;
          next = a->states[st].next[1][1][1];
          if (reachable[next] == 0) goto go_down;
        }
      }
    }
    if (reachable[st] == 1) return;
    st = reachable[st]-2;
    continue;
go_down:
    reachable[next] = st+2;
    st = next;
  }
}

void automaton_print(
  FILE              *file,
  const settings_t  *settings,
  const automaton_t *a)
{
  int i;
  unsigned short *reachable = malloc(sizeof(unsigned short) * a->state_n);
  memset(reachable, 0, sizeof(unsigned short) * a->state_n);

  find_reachable_states(a, settings, reachable);

  fprintf(file, "digraph automaton {\n");
  fprintf(file, "  node [shape = doublecircle, label = \"S%0.3f\"] ST_0;\n",
    (float)a->states[0].action / ACTION_RESOLUTION);
  for (i = 1; i < a->state_n; ++i) {
    if ((settings->flags & F_SHOW_UNREACHABLE) == 0 && !reachable[i]) {
      continue;
    }
    fprintf(file, "  node [shape = circle, label = \"%0.3f\"] ST_%d;\n",
      (float)a->states[i].action / ACTION_RESOLUTION,
      i);
  }
  for (i = 0; i < a->state_n; ++i) {
    if ((settings->flags & F_SHOW_UNREACHABLE) == 0 && !reachable[i]) {
      continue;
    }
    if ((settings->flags & F_MOVE_AWARE) == 0) {
      fprintf(file, "  ST_%d -> ST_%d [label = \"@0\"];\n",
        i, (int)a->states[i].next[0][0][0]);
      fprintf(file, "  ST_%d -> ST_%d [label = \"@1\"];\n",
        i, (int)a->states[i].next[0][0][1]);
      if ((settings->flags & F_MISTAKE_AWARE)
        && settings->mistake_rate > 0.0)
      {
        fprintf(file, "  ST_%d -> ST_%d [label = \"#0\"];\n",
          i, (int)a->states[i].next[1][0][0]);
        fprintf(file, "  ST_%d -> ST_%d [label = \"#1\"];\n",
          i, (int)a->states[i].next[1][0][1]);
      }
    } else {
      if (a->states[i].action != 0) {
        fprintf(file, "  ST_%d -> ST_%d [label = \"@10\"];\n",
          i, (int)a->states[i].next[0][1][0]);
        fprintf(file, "  ST_%d -> ST_%d [label = \"@11\"];\n",
          i, (int)a->states[i].next[0][1][1]);
        if ((settings->flags & F_MISTAKE_AWARE)
          && settings->mistake_rate > 0.0)
        {
          fprintf(file, "  ST_%d -> ST_%d [label = \"#00\"];\n",
            i, (int)a->states[i].next[1][0][0]);
          fprintf(file, "  ST_%d -> ST_%d [label = \"#01\"];\n",
            i, (int)a->states[i].next[1][0][1]);
        }
      }
      if (a->states[i].action != ACTION_RESOLUTION) {
        fprintf(file, "  ST_%d -> ST_%d [label = \"@00\"];\n",
          i, (int)a->states[i].next[0][0][0]);
        fprintf(file, "  ST_%d -> ST_%d [label = \"@01\"];\n",
          i, (int)a->states[i].next[0][0][1]);
        if ((settings->flags & F_MISTAKE_AWARE)
          && settings->mistake_rate > 0.0)
        {
          fprintf(file, "  ST_%d -> ST_%d [label = \"#10\"];\n",
            i, (int)a->states[i].next[1][1][0]);
          fprintf(file, "  ST_%d -> ST_%d [label = \"#11\"];\n",
            i, (int)a->states[i].next[1][1][1]);
        }
      }
    }
  }
  fprintf(file, "}\n");

  free(reachable);
}

static void state_serialize(FILE *file, const state_t *st) {
  fprintf(file, "#STATE\n");
  fprintf(file, "action=%hu\n", st->action);
  fprintf(file, "next=");
  for (int i = 0; i < 8; ++i) {
    fprintf(file, " %hu", st->next_tab[i]);
  }
  fprintf(file, "\n");
}

void automaton_serialize(FILE *file, const automaton_t *a) {
  fprintf(file, "#AUTOMATON\n");
  fprintf(file, "state_n=%hu\n", a->state_n);
  fprintf(file, "lifetime=%hu\n", a->lifetime);
  fprintf(file, "color=%x\n", a->color);
  for (int i = 0; i < a->state_n; ++i) {
    state_serialize(file, & a->states[i]);
  }
}
