#!/bin/bash

g++ -o Programmer Programmer.cpp common.cpp
g++ -o Server Server.cpp common.cpp

./Server &
./Programmer &
./Programmer &
./Programmer &
wait