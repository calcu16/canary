#include "gminor.h"
#include <setjmp.h>
#include <string.h>
#include "bitset.h"
#include "constants.h"
#include "graph.h"

struct context {
  /* a graph and its minor */
  const struct graph *g, *h;

  /* current traversed path */
  struct bitset path;
  /* neighbors of the current branch */
  struct bitset neighbors;
  
  /* vertices that are unassigned */
  struct bitset unassigned;
  /* vertices that have two assignments */
  struct bitset undecided;
  /* assignments of vertices in g to a vertex in h (index) */
  struct bitset assigned[MAX_VERTICES];

  /* partial path from hs to he through gv (index) */
  struct bitset partial_paths[MAX_VERTICES];
  /* hs */
  int path_start_vertex[MAX_VERTICES];
  /* he */
  int path_end_vertex[MAX_VERTICES];

  /* full path from hs to he */
  struct bitset full_paths[MAX_VERTICES][MAX_VERTICES];

  /* used to longjmp back to the beginning */
  jmp_buf top;
};

static void
path(struct context * c, int hs, int he);


static void
end_path(struct context * c, int hs, int he, int gv, char first) {
  int i;

  if (!bitset_get(c->assigned[he], gv)) {
    return;
  }

  if (!bitset_get(c->undecided, gv)) {
    path(c, hs, he + 1);
    return;
  }

  c->undecided = bitset_remove(c->undecided, gv);
  c->assigned[c->path_start_vertex[gv]] = bitset_remove(c->assigned[c->path_start_vertex[gv]], gv);
  c->assigned[c->path_end_vertex[gv]] = bitset_remove(c->assigned[c->path_end_vertex[gv]], gv);
  c->assigned[he] = bitset_add(c->assigned[he], gv);
 
  for (i = 0; i < c->g->n; ++i) {
    if (bitset_get(c->g->m[gv], i)) {
      end_path(c, hs, he, i, 0);
    }
  }

  c->assigned[c->path_end_vertex[gv]] = bitset_add(c->assigned[c->path_end_vertex[gv]], gv);
  c->assigned[c->path_start_vertex[gv]] = bitset_add(c->assigned[c->path_start_vertex[gv]], gv);
  c->undecided = bitset_add(c->undecided, gv);
}

static void
unassigned_path(struct context * c, int hs, int he, int gv, char first) {
  int i;

  if (!bitset_get(c->unassigned, gv)) {
    end_path(c, hs, he, gv, first);
    return;
  }

  c->unassigned = bitset_remove(c->unassigned, gv);
  c->undecided = bitset_add(c->undecided, gv);
  c->assigned[hs] = bitset_add(c->assigned[hs], gv);
  c->assigned[he] = bitset_add(c->assigned[he], gv);
  c->path_start_vertex[gv] = hs;
  c->path_end_vertex[gv] = he;

  for (i = 0; i < c->g->n; ++i) {
    if (bitset_get(c->g->m[gv], i)) {
      unassigned_path(c, hs, he, i, 0);
    }
  }

  c->assigned[he] = bitset_remove(c->assigned[he], gv);
  c->assigned[hs] = bitset_remove(c->assigned[hs], gv);
  c->undecided = bitset_remove(c->undecided, gv);
  c->unassigned = bitset_add(c->unassigned, gv);
}

static void
start_path(struct context * c, int hs, int he, int gv, char first) {
  int i;
  struct bitset old_neighbors;

  if (!bitset_get(c->h->m[hs], gv)) {
    unassigned_path(c, hs, he, gv, first);
    return;
  }
  if (!bitset_get(c->undecided, gv)) {
    return;
  }

  c->undecided = bitset_remove(c->undecided, gv);
  c->assigned[c->path_end_vertex[gv]] = bitset_remove(c->assigned[c->path_end_vertex[gv]], gv);
  old_neighbors = c->neighbors;
  c->neighbors = bitset_or(c->neighbors, c->g->m[gv]);

  for (i = 0; i < c->g->n; ++i) {
    if (bitset_get(c->g->m[gv], i)) {
      start_path(c, hs, he, i, 0);
    }
  }
  
  c->neighbors = old_neighbors;
  c->assigned[c->path_end_vertex[gv]] = bitset_add(c->assigned[c->path_end_vertex[gv]], gv);
  c->undecided = bitset_add(c->undecided, gv);
}

static void
assign(struct context * c, int hv) {
  int i;
  if (hv == c->h->n) {
    _longjmp(c->top, 1);
  }
  for (i = 0; i < c->g->n; ++i) {
    if (bitset_get(c->unassigned, i)) {
      c->unassigned = bitset_remove(c->unassigned, i);
      c->assigned[hv] = bitset_add(c->assigned[hv], i);
      c->neighbors = c->g->m[i];

      path(c, i, 0);

      c->assigned[hv] = bitset_remove(c->assigned[hv], i);
      c->unassigned = bitset_add(c->assigned[hv], i);
    }
  }
}

static void
path(struct context * c, int hs, int he) {
  int i; 

  for (;hs < he && !bitset_get(c->h->m[hs], he); ++he) ;
  if (hs == he) {
    assign(c, hs + 1);
    return;
  }

  for (i = 0; i < c->g->n; ++i) {
    if (bitset_get(c->neighbors, i)) {
      start_path(c, hs, he, i, 1);
    }
  }
}

char
is_minor(const struct graph * g, const struct graph * h) {
  struct context c;

  memset(&c, 0, sizeof(c));
  if (_setjmp(c.top)) {
    return 1;
  }
  c.unassigned = bitset_all();
  assign(&c, 0);
  return 0;
}

