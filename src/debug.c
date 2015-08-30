#include "debug.h"
#include <sys/time.h>

enum debug log_level = OFF;

uint64_t microseconds(void) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec;
}
