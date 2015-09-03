#ifndef _G6_H_
#define _G6_H_
struct graph;

void
atog(const char g6[], struct graph * g);

void
print_adjacency_list(const struct graph * g);
#endif/*_G6_H_*/
