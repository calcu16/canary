#include "gminor.h"
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "bitset.h"
#include "constants.h"
#include "debug.h"
#include "graph.h"

struct context {
  /* a graph and its minor */
  const struct graph *g, *h;

  /* current traversed path */
  struct bitset path;
  
  /* vertices that are unassigned */
  struct bitset unassigned;
  /* vertices that have two assignments */
  struct bitset undecided;
  /* assignments of vertices in g to a vertex in h (index) or to another */
  struct bitset half_assigned[MAX_VERTICES];
  /* assignments of vertices in g to a vertex in h (index) */
  struct bitset assigned[MAX_VERTICES];

  /* used to longjmp back to the beginning */
  jmp_buf top;
};

static void
path(struct context * c, int hs, int he);


static void
end_path(struct context * c, int hs, int he, int gv, char first) {
  int i;

  if (!bitset_get(c->half_assigned[he], gv) || bitset_get(c->path, gv)) {
    return;
  }
  if (bitset_get(c->assigned[he], gv)) {
    c->assigned[hs] = bitset_or(c->assigned[hs], bitset_and(c->half_assigned[hs], c->path));
    c->assigned[he] = bitset_or(c->assigned[he], bitset_and(c->half_assigned[he], c->path));
    c->half_assigned[hs] = bitset_or(c->half_assigned[hs], bitset_and(c->unassigned, c->path));
    c->half_assigned[he] = bitset_or(c->half_assigned[he], bitset_and(c->unassigned, c->path));
    c->undecided = bitset_or(bitset_minus(c->undecided, c->path), bitset_and(c->unassigned, c->path));
    c->unassigned = bitset_minus(c->unassigned, c->path);

    path(c, hs, he + 1);

    c->unassigned = bitset_or(c->unassigned, bitset_and(c->undecided, c->path));
    c->undecided = bitset_minus(bitset_or(c->undecided, c->path), c->unassigned);
    c->half_assigned[he] = bitset_minus(c->half_assigned[he], c->unassigned);
    c->half_assigned[hs] = bitset_minus(c->half_assigned[hs], c->unassigned);
    c->assigned[he] = bitset_minus(c->assigned[he], c->path);
    c->assigned[hs] = bitset_minus(c->assigned[hs], c->path);
    return;
  }
  if (!bitset_get(c->undecided, gv)) {
    return;
  }

  c->path = bitset_add(c->path, gv);
  for (i = 0; i < c->g->n; ++i) {
    if (bitset_equal(bitset_and(c->g->m[i], c->path), bitset_single(i))) {
      end_path(c, hs, he, i, 0);
    }
  }
  c->path = bitset_remove(c->path, gv);
}

static void
unassigned_path(struct context * c, int hs, int he, int gv, char first) {
  int i;

  if (bitset_get(c->path, gv)) {
    return;
  }
  if (!bitset_get(c->unassigned, gv)) {
    end_path(c, hs, he, gv, first);
    return;
  }

  c->path = bitset_add(c->path, gv);
  for (i = 0; i < c->g->n; ++i) {
    if (bitset_equal(bitset_and(c->g->m[i], c->path), bitset_single(i))) {
      unassigned_path(c, hs, he, i, 0);
    }
  }
  c->path = bitset_remove(c->path, gv);
}

static void
start_path(struct context * c, int hs, int he, int gv, char first) {
  int i;
  if (bitset_get(c->path, gv)) {
    return;
  }
  if (!bitset_get(c->half_assigned[hs], gv)) {
    unassigned_path(c, hs, he, gv, first);
    return;
  }
  if (!bitset_get(c->undecided, gv)) {
    return;
  }

  c->path = bitset_add(c->path, gv);
  for (i = 0; i < c->g->n; ++i) {
    if (bitset_equal(bitset_and(c->g->m[i], c->path), bitset_single(i))) {
      start_path(c, hs, he, i, 0);
    }
  }
  c->path = bitset_remove(c->path, gv);
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
      c->half_assigned[hv] = bitset_add(c->half_assigned[hv], i);
      c->assigned[hv] = bitset_add(c->assigned[hv], i);
      
      path(c, hv, 0);

      c->assigned[hv] = bitset_remove(c->assigned[hv], i);
      c->half_assigned[hv] = bitset_remove(c->half_assigned[hv], i);
      c->unassigned = bitset_add(c->unassigned, i);
    }
  }
}

static void
path(struct context * c, int hs, int he) {
  int i; 
  struct bitset old_path;

  for (;he < hs && !bitset_get(c->h->m[hs], he); ++he) ;
  if (hs == he) {
    assign(c, hs + 1);
    return;
  }
  old_path = c->path;
  c->path = bitset_empty();
  for (i = 0; i < c->g->n; ++i) {
    if (!bitset_isempty(bitset_and(c->g->m[i], c->assigned[hs]))) {
      start_path(c, hs, he, i, 1);
    }
  }
  c->path = old_path;
}

static void
log_context(struct context * c, enum debug level) {
  int i;
  LOGF(level, "graph of size %d, minor of size %d\n", c->g->n, c->h->n);
  LOGF(level, "unassigned : %llx\n", c->unassigned.v);
  LOGF(level, "undecided : %llx\n", c->undecided.v);
  for (i = 0; i < c->h->n; ++i) {
    LOGF(level, "half assigned to %d: %llx\n", i, c->assigned[i].v);
  }
  for (i = 0; i < c->h->n; ++i) {
    LOGF(level, "assigned to %d: %llx\n", i, c->assigned[i].v);
  }
}

char
is_minor(const struct graph * g, const struct graph * h) {
  struct context c;

  memset(&c, 0, sizeof(c));
  if (_setjmp(c.top)) {
    log_context(&c, INFO);
    return 1;
  }
  c.unassigned = bitset_all();
  c.g = g;
  c.h = h;
  assign(&c, 0);
  return 0;
}

