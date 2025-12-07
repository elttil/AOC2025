#!/bin/bash
clang -O2 main.c -o ./day07 || exit 1
time cat input.txt | ./day07
