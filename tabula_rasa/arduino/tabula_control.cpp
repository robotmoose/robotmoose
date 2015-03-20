/**
 Scalable command and sense system for robot control:

command: PC sending control commands for motor controllers, 
  enable bits, etc.

sensor: Arduino sending temperature, ultrasonic, bumpers, etc.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-18 (Public Domain)
*/
#include "tabula_control.h"
#include "tabula_config.h"

// Static value storage:
tabula_control_storage tabula_sensor_storage;
tabula_control_storage tabula_command_storage;

// Example devices moved to devices.cpp
