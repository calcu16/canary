#define MAIN
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "debug.h"
#include "g6.h"
#include "graph.h"
#include "gminor.h"

static const char usage[] =
  "%s [--log=OFF|FATAL|ERROR|WARN|INFO|DEBUG|TRACE] [--any|all] MINOR ... -- GRAPH ...]\n"
  "  Tests each graph for all given minors,\n"
  "  by default prints out the cross product of the results\n"
  "\n"
  "  --any print out one result if a graph has any of the minors\n"
  "  --all print out one result if a graph has all of the minors\n";

enum option {
  NONE,
  ANY,
  ALL
};

enum sort {
  NOSORT,
  FORWARD,
  REVERSE
};


static void test_one(const struct graph * g, const char *hs[], int hl, enum option o, enum sort hsort) {
  struct graph h;
  int i;
  int r;
  for (i = 0; i < hl; ++i) {
    atog(hs[i], &h);
    if (hsort != NOSORT) {
      graph_sort(&h, hsort == FORWARD);
    }
    r = is_minor(g, &h);
    if (r && o == ANY) {
      printf("1\n");
      return;
    } else if (!r && o == ALL) {
      printf("0\n");
      return;
    } else if (o == NONE) {
      if (i != 0) {
        printf(" ");
      }
     printf("%d", r);
    }
  }
  switch (o) {
  case ANY: printf("0\n"); return;
  case ALL: printf("1\n"); return;
  case NONE: printf("\n"); return;
  }
}

static void test_all(const char *gs[], int gl, const char *hs[], int hl, enum option o, enum sort gsort, enum sort hsort) {
  struct graph g;
  int i;
  for (i = 0; i < gl; ++i) {
    atog(gs[i], &g);
    if (gsort != NOSORT) {
      graph_sort(&g, gsort == FORWARD);
    }
    test_one(&g, hs, hl, o, hsort);
  }
}

int
main(int argc, const char *argv[]) {
  char t = 0;
  int gs, gl, hs = 1, hl;
  enum option o = NONE;
  enum sort gsort = NOSORT, hsort = NOSORT;
  uint64_t start, end, delta;

  if (argc == 1) {
    fprintf(stderr, usage, argv[0]);
    exit(1);
  }

  if (!strncmp(argv[hs], "--log=", strlen("--log="))) {
    switch (argv[hs][strlen("--log=")]) {
    case 'o': log_level = OFF; break;
    case 'f': log_level = FATAL; break;
    case 'e': log_level = ERROR; break;
    case 'w': log_level = WARN; break;
    case 'i': log_level = INFO; break;
    case 'd': log_level = DEBUG; break;
    case 't': log_level = TRACE; break;
    default:
      fprintf(stderr, usage, argv[0]);
      exit(1);
    }
    ++hs;
  }

  if (!strncmp(argv[hs], "--time", strlen("--time"))) {
    t = 1;
    ++hs;
  }

  if (!strcmp(argv[hs], "--any") || !strcmp(argv[hs], "--all")) {
    if (!strcmp(argv[hs], "--any")) {
      o = ANY;
    } else {
      o = ALL;
    }
    ++hs;
  }

  if (!strcmp(argv[hs], "--gsort") || !strcmp(argv[hs], "--grsort")) {
    if (!strcmp(argv[hs], "--gsort")) {
      gsort = FORWARD;
    } else {
      gsort = REVERSE;
    }
    ++hs;
  }

  if (!strcmp(argv[hs], "--hsort") || !strcmp(argv[hs], "--hrsort")) {
    if (!strcmp(argv[hs], "--hsort")) {
      hsort = FORWARD;
    } else {
      hsort = REVERSE;
    }
    ++hs;
  }

  for (hl = 0; hs + hl < argc && strcmp(argv[hs + hl], "--"); ++hl) ;
  if (hs + hl == argc) {
    fprintf(stderr, usage, argv[0]);
    exit(1);
  }
  gs = hs + hl + 1;
  gl = argc - gs;

  start = microseconds();
  test_all(&argv[gs], gl, &argv[hs], hl, o, gsort, hsort);
  end = microseconds();
  delta = end - start;
  if (t) {
    printf("Completed request in %llu.%06llu seconds\n", delta / 1000000, delta % 1000000);
  }
  return 0;
}
