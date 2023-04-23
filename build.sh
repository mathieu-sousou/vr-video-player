#!/bin/sh -e

dependencies="glm glew sdl2 openvr x11 xcomposite xfixes mpv"
includes=$(pkg-config --cflags $dependencies)
libs="$(pkg-config --libs $dependencies) -lm -pthread"
gcc -c src/window_texture.c -O2 -DNDEBUG $includes
g++ -c src/mpv.cpp -O2 -DNDEBUG $includes
g++ -c src/main.cpp -O2 -DNDEBUG $includes
g++ -o vr-video-player -O2 window_texture.o mpv.o main.o -s $libs
