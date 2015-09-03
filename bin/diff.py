#!/bin/env python
from itertools import izip_longest
from sys import argv, stdin, exit

if len(argv) != 3:
  print "%s FILE FILE", argv[0]
  exit(1)

def open2(fn):
  if fn == "-":
    return stdin
  return open(fn)

l, r = open2(argv[1]), open2(argv[2])
for (i, (ll, rl)) in enumerate(izip_longest(l, r, fillvalue=None), 1):
  if ll != rl:
    if ll and rl:
      print i, ">", ll[:-1], "<", rl[:-1]
    elif ll:
      print i, ">", ll[:-1]
    elif rl:
      print i, "<", rl[:-1]

