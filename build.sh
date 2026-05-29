#!/bin/bash
set -e
g++ -std=c++17 -c lexer.cpp    -o lexer.o
g++ -std=c++17 -c grammar.cpp  -o grammar.o
g++ -std=c++17 -c lr1.cpp      -o lr1.o
g++ -std=c++17 -c semantic.cpp -o semantic.o
g++ -std=c++17 -c parser.cpp   -o parser.o
g++ -std=c++17 -c main.cpp     -o main.o
g++ lexer.o grammar.o lr1.o semantic.o parser.o main.o -o Main