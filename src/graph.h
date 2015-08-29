#ifndef _GRAPH_H_
#define _GRAPH_H_
#include "bitset.h"
#include "constants.h"

struct graph {
  /* number of vertices */
  int n;
  /* adjacency matrix */
  struct bitset m[MAX_VERTICES];
};

inline static void
graph_add_edge(struct graph * g, int i, int j, char v) {
  g->m[i] = bitset_add_value(g->m[i], j, !!v);
}

inline static char
graph_has_edge(struct graph * g, int i, int j) {
  return bitset_get(g->m[i], j);
}
#endif/*_GRAPH_H_*/
