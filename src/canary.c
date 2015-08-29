#define MAIN
#include <stdio.h> 
#include <stdlib.h> 
#include "g6.h"
#include "graph.h"
#include "gminor.h"

int
main(int argc, char *argv[]) {
  struct graph g, h;

  if (argc != 3) {
    fprintf(stderr, "%s GRAPH MINOR\n", argv[0]);
    exit(1);
  }

  atog(argv[1], &g);
  atog(argv[2], &h);

  if (is_minor(&g, &h)) {
    printf("yes\n");
  } else {
    printf("no\n");
  }
  return 0;
}
