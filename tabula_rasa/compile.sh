#!/bin/bash
g++ tester.cpp arduino/robot.cpp arduino/motor_controller.cpp -o tester -O -std=c++11 -Wall
