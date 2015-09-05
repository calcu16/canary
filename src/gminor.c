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

static void
simple_automorphisms(const struct graph * g, int results[MAX_VERTICES]) {
  int i, j;
  for (i = 0; i < g->n; ++i) {
    results[i] = MAX_VERTICES;
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

  int initial_assignment[MAX_VERTICES + 1];

  int gsa[MAX_VERTICES];
  int hsa[MAX_VERTICES];

  /* variables used in dfs */
  enum state state;
  int hs, he[MAX_VERTICES], gv[MAX_VERTICES][MAX_VERTICES], i;
  struct bitset path[MAX_VERTICES][MAX_VERTICES];
  struct bitset start_path[MAX_VERTICES][MAX_VERTICES];

  unassigned = bitset_below(bitset_single(g->n));
  undecided = bitset_empty();
  decided = bitset_empty();
  memset(assigned, 0, sizeof(assigned));
  memset(half_assigned, 0, sizeof(half_assigned));

  simple_automorphisms(g, gsa);
  simple_automorphisms(h, hsa);
  hs = -1;
  initial_assignment[MAX_VERTICES] = g->n;

assign_down:
  ++hs;
  if (hs == h->n) {
    return 1;
  }
  initial_assignment[hs] = 0;
assign_next:
  if (initial_assignment[hs] < initial_assignment[hsa[hs]]) {
    if (bitset_get(unassigned, initial_assignment[hs])) {
      unassigned = bitset_remove(unassigned, initial_assignment[hs]);
      half_assigned[hs] = bitset_add(half_assigned[hs], initial_assignment[hs]);
      assigned[hs] = bitset_add(assigned[hs], initial_assignment[hs]);
      he[hs] = -1;
      goto path_down;

assign_up:
      assigned[hs] = bitset_remove(assigned[hs], initial_assignment[hs]);
      half_assigned[hs] = bitset_remove(half_assigned[hs], initial_assignment[hs]);
      unassigned = bitset_add(unassigned, initial_assignment[hs]);
    }
    ++initial_assignment[hs];
    goto assign_next;
  }
  --hs;
  if (hs == -1) {
    return 0;
  }
  goto path_up;

path_commit:
  assigned[hs] = bitset_or(assigned[hs], bitset_and(half_assigned[hs], path[hs][he[hs]]));
  assigned[he[hs]] = bitset_or(bitset_or(assigned[he[hs]], bitset_and(half_assigned[he[hs]], path[hs][he[hs]])), bitset_minus(path[hs][he[hs]], start_path[hs][he[hs]]));
  half_assigned[hs] = bitset_or(half_assigned[hs], bitset_and(unassigned, bitset_and(path[hs][he[hs]], start_path[hs][he[hs]])));
  half_assigned[he[hs]] = bitset_or(half_assigned[he[hs]], bitset_and(unassigned, path[hs][he[hs]]));
  decided = bitset_and(bitset_minus(path[hs][he[hs]], start_path[hs][he[hs]]), unassigned);
  undecided = bitset_or(bitset_minus(undecided, path[hs][he[hs]]), bitset_and(unassigned, path[hs][he[hs]]));
  unassigned = bitset_minus(unassigned, path[hs][he[hs]]);

path_down:
  for (;++he[hs] < hs && !bitset_get(h->m[hs], he[hs]); ) ;
  if (hs == he[hs]) {
    goto assign_down;
  }
  path[hs][he[hs]] = bitset_empty();
  start_path[hs][he[hs]] = bitset_all();
  for (gv[hs][he[hs]] = initial_assignment[he[hs]]; gv[hs][he[hs]] < g->n; ++gv[hs][he[hs]]) {
    if (!bitset_isempty(bitset_and(g->m[gv[hs][he[hs]]], assigned[hs])) && bitset_get(assigned[he[hs]], gv[hs][he[hs]])) {
      goto path_down;
    }
  }
  gv[hs][he[hs]] = initial_assignment[he[hs]];
path_next:
  ++gv[hs][he[hs]];
  if (gv[hs][he[hs]] < g->n) {
    if (!bitset_isempty(bitset_and(g->m[gv[hs][he[hs]]], assigned[hs]))) {
      state = START;
      goto dfs_down;
    }
    goto path_next;
  }
path_up:
  for ( ; he[hs]-- > 0 && !bitset_get(h->m[hs], he[hs]); ) ;
  if (he[hs] == -1) {
    goto assign_up;
  } else {
    unassigned = bitset_or(bitset_or(unassigned, bitset_and(undecided, path[hs][he[hs]])), bitset_and(path[hs][he[hs]], decided));
    undecided = bitset_minus(bitset_or(undecided, path[hs][he[hs]]), unassigned);
    decided = bitset_minus(decided, path[hs][he[hs]]);
    half_assigned[he[hs]] = bitset_minus(half_assigned[he[hs]], unassigned);
    half_assigned[hs] = bitset_minus(half_assigned[hs], unassigned);
    assigned[he[hs]] = bitset_minus(assigned[he[hs]], path[hs][he[hs]]);
    assigned[hs] = bitset_minus(assigned[hs], path[hs][he[hs]]);
    goto dfs_unwind;
  }

dfs_down:
  if (bitset_get(path[hs][he[hs]], gv[hs][he[hs]]) || (!bitset_isempty(path[hs][he[hs]]) && !bitset_isempty(bitset_and(g->m[gv[hs][he[hs]]], assigned[hs])))) {
    goto dfs_up;
  }
  if (state == START && !bitset_get(half_assigned[hs], gv[hs][he[hs]])) {
    state = UNASSIGNED;
  }
  if (state == UNASSIGNED && !bitset_get(unassigned, gv[hs][he[hs]])) {
    state = END;
  }
  if (state == START && (!bitset_get(undecided, gv[hs][he[hs]]) ||  gv[hs][he[hs]] < initial_assignment[hs])) {
    goto dfs_up;
  }
  if (state == END && (!bitset_get(half_assigned[he[hs]], gv[hs][he[hs]]) || !bitset_get(undecided, gv[hs][he[hs]]))) {
    goto dfs_up;
  }
  if (state == UNASSIGNED && gv[hs][he[hs]] < initial_assignment[hs] && bitset_isall(start_path[hs][he[hs]])) {
    start_path[hs][he[hs]] = path[hs][he[hs]];
  }

  path[hs][he[hs]] = bitset_add(path[hs][he[hs]], gv[hs][he[hs]]);
  if (!bitset_isempty(bitset_and(g->m[gv[hs][he[hs]]], assigned[he[hs]]))) {
    goto path_commit;
  }
  i = initial_assignment[he[hs]] - 1;
dfs_next:
  ++i;
  if (i < g->n) {
    if (bitset_equal(bitset_and(g->m[i], path[hs][he[hs]]), bitset_single(gv[hs][he[hs]]))) {
      gv[hs][he[hs]] = i;
      goto dfs_down;
    }
    goto dfs_next;
  }
dfs_unwind:
  path[hs][he[hs]] = bitset_remove(path[hs][he[hs]], gv[hs][he[hs]]);
  if (bitset_equal(start_path[hs][he[hs]], path[hs][he[hs]])) {
    start_path[hs][he[hs]] = bitset_all();
  }
dfs_up:
  if (bitset_isempty(path[hs][he[hs]])) {
    goto path_next;
  }
  i = gv[hs][he[hs]];
  gv[hs][he[hs]] = __builtin_ctzll(bitset_and(g->m[gv[hs][he[hs]]], path[hs][he[hs]]).v);
  if (bitset_get(half_assigned[hs], gv[hs][he[hs]])) {
    state = START;
  } else if(bitset_get(unassigned, gv[hs][he[hs]])) {
    state = UNASSIGNED;
  } else {
    state = END;
  }
  goto dfs_next;
  return 0;
}

