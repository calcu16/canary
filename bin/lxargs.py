#!/bin/env python
from os import execvp
from sys import argv, stdin, stderr

args = argv[1:] + [l.rstrip() for l in stdin]
execvp(argv[1], args)
