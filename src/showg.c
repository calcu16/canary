#define MAIN
#include <stdio.h> 
#include <stdlib.h> 
#include "g6.h"
#include "graph.h"

int
main(int argc, char *argv[]) {
  int i = 0;
  struct graph g;

  if (argc == 1) {
    fprintf(stderr, "%s GRAPH MINOR\n", argv[0]);
    exit(1);
  }
  
  for (i = 1; i < argc; ++i) {
    atog(argv[i], &g);
    printf("\nGraph %d, order %d.\n", i, g.n);
    print_adjacency_list(&g);
  }
  return 0;
}
