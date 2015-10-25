#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdint.h>
#include <stdio.h>

enum debug {
  OFF,
  FATAL,
  ERROR,
  WARN,
  INFO,
  DEBUG,
  TRACE
};

#define LOG(level, msg)                                                      \
  do {                                                                       \
    if (level <= log_level) {                                                \
      printf("%llu: " msg, microseconds());                                  \
    }                                                                        \
  } while(0)

#define LOGF(level, msg, ...)                                                \
  do {                                                                       \
    if (level <= log_level) {                                                \
      printf("%llu: " msg, microseconds(), __VA_ARGS__);                     \
    }                                                                        \
  } while(0)

extern enum debug log_level;

uint64_t microseconds(void);

#define TRACE(v) trace(__func__, __LINE__, #v, v)

static inline int trace(const char * func, int line, const char * vname, long long v) {
#ifndef NDEBUG
  LOGF(TRACE, "TRACING %s.%d %s = %llu\n", func, line, vname, v);
#endif
  return 1;
}

#endif/*_DEBUG_H_*/
