#!/bin/bash
set -e
g++ -std=c++17 -c ir.cpp          -o ir.o
g++ -std=c++17 -c parse_input.cpp -o parse_input.o
g++ -std=c++17 -c blocks.cpp      -o blocks.o
g++ -std=c++17 -c liveness.cpp    -o liveness.o
g++ -std=c++17 -c regalloc.cpp    -o regalloc.o
g++ -std=c++17 -c codegen.cpp     -o codegen.o
g++ -std=c++17 -c main.cpp        -o main.o
g++ ir.o parse_input.o blocks.o liveness.o regalloc.o codegen.o main.o -o Main