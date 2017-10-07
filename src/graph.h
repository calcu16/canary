#ifndef _GRAPH_H_
#define _GRAPH_H_
#include "bitset.h"
#include "constants.h"

struct graph {
  /* number of vertices */
  int n;
  /* adjacency matrix */
  struct bitset m[MAX_VERTICES];
  /* adjacency list */
  int l[MAX_VERTICES][MAX_VERTICES];
  int ll[MAX_VERTICES];
};

inline static void
graph_add_edge(struct graph * g, int i, int j, char v) {
  g->m[i] = bitset_add_value(g->m[i], j, !!v);
  g->l[i][g->ll[i]] = j;
  g->ll[i] += !!v;
}

inline static char
graph_has_edge(const struct graph * g, int i, int j) {
  return bitset_get(g->m[i], j);
}

inline static void
graph_swap(struct graph * g, int i, int j) {
  struct bitset t = g->m[i];
  int k;
  if (i == j) {
    return;
  }
  g->m[i] = g->m[j];
  g->m[j] = t;
  for (k = 0; k < g->n; ++k) {
    if (k != i && k != j) {
      if (graph_has_edge(g, j, k)) {
        g->m[k] = bitset_add(g->m[k], j);
      } else {
        g->m[k] = bitset_remove(g->m[k], j);
      }

      if (graph_has_edge(g, i, k)) {
        g->m[k] = bitset_add(g->m[k], i);
      } else {
        g->m[k] = bitset_remove(g->m[k], i);
      }
    }
  }
  if (graph_has_edge(g, j, j)) {
    g->m[i] = bitset_remove(g->m[i], i);
    g->m[j] = bitset_remove(g->m[j], j);
    g->m[i] = bitset_add(g->m[i], j);
    g->m[j] = bitset_add(g->m[j], i);
  }
}

inline static void
graph_sort(struct graph * g, char most) {
  int i, j, c, e;
  for (i = 0; i < g->n; ++i) {
    int m = i, mc = 0, me = 0;
    for (j = i; j < g->n; ++j) {
      c = bitset_size(bitset_and(g->m[j], bitset_below(bitset_single(i))));
      e = bitset_size(g->m[j]);
      if ((c > mc || (c == mc && e > me)) == most) {
        m = j;
        mc = c;
        me = e;
      }
    }
    graph_swap(g, i, m);
  }
}
#endif/*_GRAPH_H_*/
