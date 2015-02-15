#!/bin/sh
g++ main.cpp arduino/roomba.cpp ../include/msl/serial.cpp ../include/msl/time.cpp -O -Wall -o roomba -std=c++11 -I../include
