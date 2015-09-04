#include "gminor.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "bitset.h"
#include "constants.h"
#include "debug.h"
#include "graph.h"

enum state {
  START,
  UNASSIGNED,
  END
};

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

  int gsa[MAX_VERTICES];
  int hsa[MAX_VERTICES];

  /* used to longjmp back to the beginning */
  jmp_buf top;

  /* variables used in dfs */
  enum state state;
  int hs, he, gv, i;
};

static void
path(struct context * c);

static inline void
commit_path(struct context * c) {
  c->assigned[c->hs] = bitset_or(c->assigned[c->hs], bitset_and(c->half_assigned[c->hs], c->path));
  c->assigned[c->he] = bitset_or(bitset_or(c->assigned[c->he], bitset_and(c->half_assigned[c->he], c->path)), bitset_minus(c->path, c->start_path));
  c->half_assigned[c->hs] = bitset_or(c->half_assigned[c->hs], bitset_and(c->unassigned, bitset_and(c->path, c->start_path)));
  c->half_assigned[c->he] = bitset_or(c->half_assigned[c->he], bitset_and(c->unassigned, c->path));
  c->decided = bitset_and(bitset_minus(c->path, c->start_path), c->unassigned);
  c->undecided = bitset_or(bitset_minus(c->undecided, c->path), bitset_and(c->unassigned, c->path));
  c->unassigned = bitset_minus(c->unassigned, c->path);
  ++c->he;
}

static inline void
uncommit_path(struct context * c) {
  --c->he;
  c->unassigned = bitset_or(bitset_or(c->unassigned, bitset_and(c->undecided, c->path)), bitset_and(c->path, c->decided));
  c->undecided = bitset_minus(bitset_or(c->undecided, c->path), c->unassigned);
  c->decided = bitset_minus(c->decided, c->path);
  c->half_assigned[c->he] = bitset_minus(c->half_assigned[c->he], c->unassigned);
  c->half_assigned[c->hs] = bitset_minus(c->half_assigned[c->hs], c->unassigned);
  c->assigned[c->he] = bitset_minus(c->assigned[c->he], c->path);
  c->assigned[c->hs] = bitset_minus(c->assigned[c->hs], c->path);
}

static inline void
unwind(struct context * c) {
  c->i = c->gv;
  c->gv = __builtin_ctzll(bitset_and(c->g->m[c->gv], c->path).v);
  if (bitset_get(c->half_assigned[c->hs], c->gv)) {
    c->state = START;
  } else if(bitset_get(c->unassigned, c->gv)) {
    c->state = UNASSIGNED;
  } else {
    c->state = END;
  }
}

static void
dfs(struct context * c) {
dfs_down:
  if (bitset_get(c->path, c->gv) || (!bitset_isempty(c->path) && !bitset_isempty(bitset_and(c->g->m[c->gv], c->assigned[c->hs])))) {
    goto dfs_up;
  }
  if (c->state == START && !bitset_get(c->half_assigned[c->hs], c->gv)) {
    c->state = UNASSIGNED;
  }
  if (c->state == UNASSIGNED && !bitset_get(c->unassigned, c->gv)) {
    c->state = END;
  }
  if (c->state == START && (!bitset_get(c->undecided, c->gv) ||  c->gv < c->initial_assignment[c->hs])) {
    goto dfs_up;
  }
  if (c->state == END && (!bitset_get(c->half_assigned[c->he], c->gv) || !bitset_get(c->undecided, c->gv))) {
    goto dfs_up;
  }
  if (c->state == UNASSIGNED && c->gv < c->initial_assignment[c->hs] && bitset_isall(c->start_path)) {
    c->start_path = c->path;
  }

  c->path = bitset_add(c->path, c->gv);
  if (!bitset_isempty(bitset_and(c->g->m[c->gv], c->assigned[c->he]))) {
    commit_path(c);
    path(c);
    uncommit_path(c);
  } else {
    c->i = c->initial_assignment[c->he] - 1;
dfs_next:
    ++c->i;
    if (c->i < c->g->n) {
      if (bitset_equal(bitset_and(c->g->m[c->i], c->path), bitset_single(c->gv))) {
        c->gv = c->i;
        goto dfs_down;
      }
      goto dfs_next;
    }
  }

  c->path = bitset_remove(c->path, c->gv);
  if (bitset_equal(c->start_path, c->path)) {
    c->start_path = bitset_all();
  }
dfs_up:
  if (bitset_isempty(c->path)) {
    return;
  }
  unwind(c);
  goto dfs_next;
}

static void
assign(struct context * c) {
  int i, l = c->hsa[c->hs] == -1 ? c->g->n : c->initial_assignment[c->hsa[c->hs]], old_he;
#ifndef NDEBUG
  struct bitset aunassigned = c->unassigned, aundecided = c->undecided;
#endif/*NDEBUG*/
  if (c->hs == c->h->n) {
    _longjmp(c->top, 1);
  }
  old_he = c->he;
  for (i = 0; i < l; ++i) {
    if (bitset_get(c->unassigned, i)) {
      c->unassigned = bitset_remove(c->unassigned, i);
      c->half_assigned[c->hs] = bitset_add(c->half_assigned[c->hs], i);
      c->assigned[c->hs] = bitset_add(c->assigned[c->hs], i);
      c->initial_assignment[c->hs] = i;
      c->he = 0;
      path(c);

      c->assigned[c->hs] = bitset_remove(c->assigned[c->hs], i);
      c->half_assigned[c->hs] = bitset_remove(c->half_assigned[c->hs], i);
      c->unassigned = bitset_add(c->unassigned, i);
#ifndef NDEBUG
      assert(bitset_equal(aunassigned, c->unassigned));
      assert(bitset_equal(aundecided, c->undecided));
#endif/*NDEBUG*/
    }
  }
  c->he = old_he;
}

static void
path(struct context * c) {
  int old_he, old_gv; 
  struct bitset old_path, old_start_path;

  old_he = c->he;
  for (;c->he < c->hs && !bitset_get(c->h->m[c->hs], c->he); ++c->he) ;
  if (c->hs == c->he) {
    ++c->hs;
    assign(c);
    --c->hs;
    c->he = old_he;
    return;
  }
  old_gv = c->gv;
  old_path = c->path;
  old_start_path = c->start_path;
  c->path = bitset_empty();
  c->start_path = bitset_all();
  for (c->gv = c->initial_assignment[c->he]; c->gv < c->g->n; ++c->gv) {
    if (!bitset_isempty(bitset_and(c->g->m[c->gv], c->assigned[c->hs]))
        && bitset_get(c->assigned[c->he], c->gv)
        && (c->gsa[c->gv] == -1 || !bitset_get(c->unassigned, c->gsa[c->gv]))) {
      ++c->he;
      path(c);
      --c->he;
      goto path_end;
    }
  }
  for (c->gv = c->initial_assignment[c->he] + 1; c->gv < c->g->n; ++c->gv) {
    if (!bitset_isempty(bitset_and(c->g->m[c->gv], c->assigned[c->hs]))) {
      c->state = START;
      dfs(c);
    }
  }
path_end:
  c->gv = old_gv;
  c->path = old_path;
  c->start_path = old_start_path;
  c->he = old_he;
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
  simple_automorphisms(g, c.gsa);
  simple_automorphisms(h, c.hsa);
  assign(&c);
  return 0;
}

