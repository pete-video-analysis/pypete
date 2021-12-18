#!/bin/bash

(cd core; mkdir build; make static)
gcc -o build/pypete.o -c src/petemodule.c -I/usr/include/python3.9 -Icore/include -Lcore/build -lpete -fPIC
gcc -shared -o build/pete.so build/pypete.o -Lcore/build -lpete