#!/bin/sh
cc main.c -o ./day04 || exit 1
cat input.txt | ./day04
