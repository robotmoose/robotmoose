#include "roomba.h"

roomba_t roomba(Serial3);
int speed=50;

void setup()
{
  Serial.begin(115200);
  Serial3.begin(115200);
  roomba.setup(2);
  Serial.println("Roomba Started");
}

void loop()
{
  roomba.update();
  int command=Serial.read();

  if(command>=0)
  {
    switch(command)
    {
      case 'f':
        Serial.println("Moving Forward");
        roomba.drive(speed,speed);
        break;
      case 'b':
        Serial.println("Moving Backward");
        roomba.drive(-speed,-speed);
        break;
      case 'l':
        Serial.println("Turning Left");
        //Make it go left.
        break;
      case 'r':
        Serial.println("Turning Right");
        //Make it go right.
        break;
      case 's':
        Serial.println("Stopping");
        roomba.drive(0,0);
        break;
      default:
        Serial.println("Safe Mode");
        roomba.set_mode(roomba_t::SAFE);
        break;
    }
  }
}
