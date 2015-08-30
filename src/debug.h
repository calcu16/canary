#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdint.h>

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
      printf("%llu: " msg, microseconds());                                    \
    }                                                                        \
  } while(0)

#define LOGF(level, msg, ...)                                                \
  do {                                                                       \
    if (level <= log_level) {                                                \
      printf("%llu: " msg, microseconds(), __VA_ARGS__);                       \
    }                                                                        \
  } while(0)

extern enum debug log_level;

uint64_t microseconds(void);
#endif/*_DEBUG_H_*/
