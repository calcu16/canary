#define MAIN
#include <stdio.h> 
#include <stdlib.h> 
#include "g6.h"
#include "graph.h"

int
main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "%s GRAPH MINOR\n", argv[0]);
    exit(1);
  }
  struct graph g, m;

  atog(argv[1], &g);
  atog(argv[2], &m);

  printg(&g);
  printg(&m);
  return 0;
}
