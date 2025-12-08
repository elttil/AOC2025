#!/bin/sh
gcc -O2 main.c -o day08 || exit 1
cat input.txt | ./day08
