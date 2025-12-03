#!/bin/sh
cc main.c -o day03 || exit 1
cat input.txt | ./day03
