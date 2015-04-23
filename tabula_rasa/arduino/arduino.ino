#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>
#include "tabula_config.h"

void setup()
{
  tabula_setup(); // builds devices based on ASCII serial input.  Blocks until "loop!" command
}

void loop()
{
  action_loop(); // runs actions
}

