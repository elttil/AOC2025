#!/bin/bash
gcc -O2 main.c -o day09 || exit 1
cat input.txt | ./day09
