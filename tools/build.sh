#!/bin/sh
set -e
mkdir -p ../build
set -xe
g++ -g -Wall -Wextra -Wno-unused-variable $* -o ../build/x11 ../source/x11.cpp -lX11

# sudo apt install xorg-dev

