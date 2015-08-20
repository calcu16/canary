#include "g6.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"

void
atog(const char g6[], struct graph * g) {
  int i, j, k, x;

  memset(g, 0, sizeof(*g));

  if (*g6 == ':') {
    fprintf(stderr, "Cannot read sparse6 format\n");
    exit(1);
  }

  g->n = *g6++ - BIAS6;
  if (g->n > SMALLN) {
    for (i = 0, g->n = 0; i < 3; ++i) {
      g->n = (g->n << 6) | (*g6++ -BIAS6);
    }
  }

  if (g->n >= MAX_VERTICES) {
    fprintf(stderr, "Graph of size %d is larger than maximum size %d\n", g->n, MAX_VERTICES);
    exit(1);
  }

  for (i = 1, k = 1; i < g->n; ++i) {
    for (j = 0; j < i; ++j) {
      if (!--k) {
        k = 6;
        x = *(g6++) - BIAS6;
      }
      graph_add_edge(g, i, j, x >> 5);
      graph_add_edge(g, j, i, x >> 5);
      x <<= 1;
    }
  }
}

void
printg(struct graph * g) {
  int i, j;
  printf("graph of size %d\n", g->n);
  for (i = 0; i < g->n; ++i) {
    for (j = 0; j < g->n; ++j) {
      printf("%d ", graph_has_edge(g, i, j));
    }
    printf("\n");
  }
}
