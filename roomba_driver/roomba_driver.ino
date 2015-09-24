#include "roomba.h"

roomba_t roomba(Serial);
int speed=50;

void setup()
{
  Serial.begin(57600);
  roomba.reset();
  roomba.set_mode(roomba_t::SAFE);

  roomba.drive(speed,speed);
  delay(4000);
  roomba.drive(speed,-speed);
  delay(6000);
  roomba.drive(speed,speed);
  delay(4000);
  roomba.drive(-speed,speed);
  delay(6000);
  roomba.drive(0,0);
}

void loop()
{
}
