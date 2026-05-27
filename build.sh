#!/bin/bash
g++ -std=c++17 -c lexer.cpp -o lexer.o
g++ -std=c++17 -c main.cpp  -o main.o
g++ lexer.o main.o -o Main