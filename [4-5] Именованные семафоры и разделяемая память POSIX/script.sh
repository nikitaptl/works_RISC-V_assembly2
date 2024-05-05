#!/bin/bash

g++ -o ./Server Server.cpp common.cpp
g++ -o ./Programmer Programmer.cpp common.cpp
./Programmer & ./Server