#include "roomba.h"

roomba_t roomba(Serial);
int speed=50;

void setup()
{
  Serial.begin(57600);
  roomba.reset();
  roomba.set_mode(roomba_t::SAFE);
}

void loop()
{
  int command=Serial.read();
  
  switch(command)
  {
    case 'f':
      roomba.drive(speed,speed);
      break;
    case 'b':
      roomba.drive(-speed,-speed);
      break;
    case 'l':
      //Make it go left.
      break;
    case 'r':
      //Make it go right.
      break;
    default:
      roomba.drive(0,0);
  }
}
