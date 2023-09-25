#!/bin/sh
here=$(dirname $0)
cd $here
set -e
mkdir -p ../build
set -xe
g++ -g -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter $* -o ../build/x11 ../source/x11.cpp -lX11

# sudo apt install xorg-dev

