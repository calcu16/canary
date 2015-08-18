#!/bin/sh
find $* -name *.c |\
xargs grep -m 1 -c '^#define MAIN' /dev/null |\
sed -n "s,:1$,,p"
