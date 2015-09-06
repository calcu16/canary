#include "gminor.h"
#include <assert.h>
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
  struct bitset start_path;
  
  /* vertices that are unassigned */
  struct bitset unassigned;
  /* vertices that have two assignments */
  struct bitset undecided;
  /* vertices that have been immediately decide */
  struct bitset decided;
  /* assignments of vertices in g to a vertex in h (index) or to another */
  struct bitset half_assigned[MAX_VERTICES];
  /* assignments of vertices in g to a vertex in h (index) */
  struct bitset assigned[MAX_VERTICES];

  int initial_assignment[MAX_VERTICES];

  int sa[MAX_VERTICES];

  /* used to longjmp back to the beginning */
  jmp_buf top;
};

enum state {
  START,
  UNASSIGNED,
  END
};

static void
path(struct context * c, int hs, int he);

static void
assign_path(struct context * c, int hs, int he) {
#ifndef NDEBUG
  struct bitset apath = c->path, aunassigned = c->unassigned, aundecided = c->undecided;
#endif/*NDEBUG*/
  c->assigned[hs] = bitset_or(c->assigned[hs], bitset_and(c->half_assigned[hs], c->path));
  c->assigned[he] = bitset_or(bitset_or(c->assigned[he], bitset_and(c->half_assigned[he], c->path)), bitset_minus(c->path, c->start_path));
  c->half_assigned[hs] = bitset_or(c->half_assigned[hs], bitset_and(c->unassigned, bitset_and(c->path, c->start_path)));
  c->half_assigned[he] = bitset_or(c->half_assigned[he], bitset_and(c->unassigned, c->path));
  c->decided = bitset_and(bitset_minus(c->path, c->start_path), c->unassigned);
  c->undecided = bitset_or(bitset_minus(c->undecided, c->path), bitset_and(c->unassigned, c->path));
  c->unassigned = bitset_minus(c->unassigned, c->path);

  path(c, hs, he + 1);

  c->unassigned = bitset_or(bitset_or(c->unassigned, bitset_and(c->undecided, c->path)), bitset_and(c->path, c->decided));
  c->undecided = bitset_minus(bitset_or(c->undecided, c->path), c->unassigned);
  c->decided = bitset_minus(c->decided, c->path);
  c->half_assigned[he] = bitset_minus(c->half_assigned[he], c->unassigned);
  c->half_assigned[hs] = bitset_minus(c->half_assigned[hs], c->unassigned);
  c->assigned[he] = bitset_minus(c->assigned[he], c->path);
  c->assigned[hs] = bitset_minus(c->assigned[hs], c->path);
#ifndef NDEBUG
  assert(bitset_equal(apath, c->path));
  assert(bitset_equal(aunassigned, c->unassigned));
  assert(bitset_equal(aundecided, c->undecided));
#endif/*NDEBUG*/
}

static void
dfs(struct context * c, enum state state, int hs, int he, int gv, char first) {
  int i;
  if (bitset_get(c->path, gv) || (!first && !bitset_isempty(bitset_and(c->g->m[gv], c->assigned[hs])))) {
    return;
  }
  if (state == START && !bitset_get(c->half_assigned[hs], gv)) {
    state = UNASSIGNED;
  }
  if (state == UNASSIGNED && !bitset_get(c->unassigned, gv)) {
    state = END;
  }
  if (state == START && (!bitset_get(c->undecided, gv) ||  gv < c->initial_assignment[hs])) {
    return;
  }
  if (state == UNASSIGNED && gv < c->initial_assignment[hs] && bitset_isall(c->start_path)) {
    c->start_path = c->path;
  }
  if (state == END && (!bitset_get(c->half_assigned[he], gv) || !bitset_get(c->undecided, gv))) {
    return;
  }

  c->path = bitset_add(c->path, gv);
  if (!bitset_isempty(bitset_and(c->g->m[gv], c->assigned[he]))) {
    assign_path(c, hs, he);
  } else {
    for (i = c->initial_assignment[he]; i < c->g->n; ++i) {
      if (bitset_equal(bitset_and(c->g->m[i], c->path), bitset_single(gv))) {
        dfs(c, state, hs, he, i, 0);
      }
    }
  }
  c->path = bitset_remove(c->path, gv);
  if (bitset_equal(c->start_path, c->path)) {
    c->start_path = bitset_all();
  }
}

static void
assign(struct context * c, int hv) {
  int i, l = c->sa[hv] == -1 ? c->g->n : c->initial_assignment[c->sa[hv]];
#ifndef NDEBUG
  struct bitset aunassigned = c->unassigned, aundecided = c->undecided;
#endif/*NDEBUG*/
  if (hv == c->h->n) {
    _longjmp(c->top, 1);
  }
  for (i = 0; i < l; ++i) {
    if (bitset_get(c->unassigned, i)) {
      c->unassigned = bitset_remove(c->unassigned, i);
      c->half_assigned[hv] = bitset_add(c->half_assigned[hv], i);
      c->assigned[hv] = bitset_add(c->assigned[hv], i);
      c->initial_assignment[hv] = i;
      
      path(c, hv, 0);

      c->assigned[hv] = bitset_remove(c->assigned[hv], i);
      c->half_assigned[hv] = bitset_remove(c->half_assigned[hv], i);
      c->unassigned = bitset_add(c->unassigned, i);
#ifndef NDEBUG
      assert(bitset_equal(aunassigned, c->unassigned));
      assert(bitset_equal(aundecided, c->undecided));
#endif/*NDEBUG*/
    }
  }
}

static void
path(struct context * c, int hs, int he) {
  int i; 
  struct bitset old_path, old_start_path;

  for (;he < hs && !bitset_get(c->h->m[hs], he); ++he) ;
  if (hs == he) {
    assign(c, hs + 1);
    return;
  }
  old_path = c->path;
  old_start_path = c->start_path;
  c->path = bitset_empty();
  c->start_path = bitset_all();
  for (i = c->initial_assignment[he]; i < c->g->n; ++i) {
    if (!bitset_isempty(bitset_and(c->g->m[i], c->assigned[hs])) && bitset_get(c->assigned[he], i)) {
      path(c, hs, he + 1);
      c->path = old_path;
      c->start_path = old_start_path;
      return;
    }
  }
  for (i = c->initial_assignment[he] + 1; i < c->g->n; ++i) {
    if (!bitset_isempty(bitset_and(c->g->m[i], c->assigned[hs]))) {
      dfs(c, START, hs, he, i, 1);
    }
  }
  c->path = old_path;
  c->start_path = old_start_path;
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

static void
simple_automorphisms(const struct graph * g, int results[MAX_VERTICES]) {
  int i, j;
  for (i = 0; i < g->n; ++i) {
    results[i] = -1;
    for (j = i; j-- > 0; ) {
      if (bitset_equal(bitset_minus(g->m[i], bitset_single(j)), bitset_minus(g->m[j], bitset_single(i)))) {
        LOGF(DEBUG, "%d is automorphic to %d\n", i, j);
        results[i] = j;
        break;
      }
    }
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
  c.g = g;
  c.h = h;
  c.unassigned = bitset_below(bitset_single(g->n));
  simple_automorphisms(h, c.sa);
  assign(&c, 0);
  return 0;
}

