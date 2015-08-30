#ifndef _BITSET_H_
#define _BITSET_H_
#include <stdint.h>

/* currently assumes a 64 bit maximum graph size */
struct bitset {
  uint64_t v;
};

inline static struct bitset
bitset_empty() {
  return (struct bitset) { 0 };
}

inline static struct bitset
bitset_all() {
  return (struct bitset) { -1 };
}

inline static struct bitset
bitset_below(struct bitset v) {
  return (struct bitset) { (-v.v & v.v) - 1 };
}

inline static struct bitset
bitset_single_value(int i, uint64_t v) {
  return (struct bitset) { !!v << i };
}

inline static struct bitset
bitset_single(int i) {
  return bitset_single_value(i, 1);
}

inline static struct bitset
bitset_not(struct bitset v) {
  return (struct bitset) { ~v.v };
}

inline static struct bitset
bitset_or(struct bitset l, struct bitset r) {
  return (struct bitset) { l.v | r.v };
}

inline static struct bitset
bitset_and(struct bitset l, struct bitset r) {
  return (struct bitset) { l.v & r.v };
}

inline static struct bitset
bitset_minus(struct bitset l, struct bitset r) {
  return bitset_and(l, bitset_not(r));;
}

inline static char
bitset_isempty(struct bitset s) {
  return !s.v;
}

inline static struct bitset
bitset_add(struct bitset s, int i) {
  return bitset_or(s, bitset_single(i));
}

inline static struct bitset
bitset_add_value(struct bitset s, int i, char v) {
  return bitset_or(s, bitset_single_value(i, v));
}

inline static struct bitset
bitset_remove(struct bitset s, int i) {
  return bitset_minus(s, bitset_single(i));
}

inline static char
bitset_get(struct bitset s, int i) {
  return !bitset_isempty(bitset_and(s, bitset_single(i)));
}

inline static char
bitset_equal(struct bitset l, struct bitset r) {
  return l.v == r.v;
}
#endif/*_BITSET_H_*/
