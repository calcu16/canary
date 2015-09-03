#define MAIN
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include "debug.h"
#include "g6.h"
#include "graph.h"

int
main(int argc, char *argv[]) {
  char t = 0;
  int i = 0, j = 1;
  struct graph g;
  uint64_t start, end, delta;

  if (argc == 1) {
    fprintf(stderr, "%s GRAPH MINOR\n", argv[0]);
    exit(1);
  }

  if (!strncmp(argv[j], "--time", strlen("--time"))) {
    t = 1;
    ++j;
  }
  
  start = microseconds();
  for (i = 1; j < argc; ++j, ++i) {
    atog(argv[j], &g);
    printf("\nGraph %d, order %d.\n", i, g.n);
    print_adjacency_list(&g);
  }
  end = microseconds();
  delta = end - start;
  if (t) {
    printf("Completed request in %llu.%06llu seconds\n", delta / 1000000, delta % 1000000);
  }
  return 0;
}
