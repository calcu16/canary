#ifndef _BITSET_H_
#define _BITSET_H_
#include <stdint.h>

/* currently assumes a 64 bit maximum graph size */
struct bitset {
  uint64_t v;
};

inline static
struct bitset
bitset_empty() {
  return (struct bitset) { 0 };
}

inline static
struct bitset
bitset_all() {
  return (struct bitset) { -1 };
}

inline static
struct bitset
bitset_single(int i) {
  return (struct bitset) { 1ULL << i };
}

inline static
struct bitset
bitset_or(struct bitset l, struct bitset r) {
  return (struct bitset) { l.v | r.v };
}

inline static
struct bitset
bitset_and(struct bitset l, struct bitset r) {
  return (struct bitset) { l.v & r.v };
}

inline static
char
bitset_isempty(struct bitset s) {
  return !s.v;
}

inline static
struct bitset
bitset_add(struct bitset s, int i, char v) {
  return bitset_or(s, (struct bitset) { ((uint64_t) !!v) << i});
}

inline static
char
bitset_get(struct bitset s, int i) {
  return bitset_isempty(bitset_and(s, bitset_single(i)));
}
#endif/*_BITSET_H_*/
