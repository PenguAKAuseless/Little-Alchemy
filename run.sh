#!/usr/bin/bash

g++ -c main.cpp -o main.o -std=c++17
g++ main.o -o game -lsfml-graphics -lsfml-window -lsfml-system
./game