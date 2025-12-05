#!/bin/sh
gcc -O2 main.c -o day05 || exit 1
cat input.txt | ./day05
