#!/bin/sh
find $* -name *.c |\
xargs grep -c '^#define MAIN' /dev/null |\
sed -n "s,:[^0]$,,p"
