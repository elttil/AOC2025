#!/bin/sh
gcc -O2 main.c -o day06 || exit 1
cat input.txt | ./day06
