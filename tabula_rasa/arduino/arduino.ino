#include "motor_controller.h"

uint16_t right_pins[2]={3,5};
uint16_t left_pins[2]={9,6};

bts_controller_t test(left_pins,right_pins);

void setup()
{
  test.setup();
  
  for(int ii=16;ii<=19;++ii)
  {
    pinMode(ii,OUTPUT);
    digitalWrite(ii,HIGH);
  }
}

void loop()
{
  test.drive(0,0);
}


