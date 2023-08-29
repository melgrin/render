#!/bin/sh
set -e
mkdir -p ../build
set -xe
gcc -g -Wall -Wextra --pedantic -o ../build/x11 ../source/x11.c -lX11

# sudo apt install xorg-dev

