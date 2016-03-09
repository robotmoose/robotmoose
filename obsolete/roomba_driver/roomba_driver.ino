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

  Serial.println("Moving Forward");
  roomba.drive(speed,speed);
  delay(4000);

  Serial.println("Turning Right");
  roomba.drive(speed,-speed);
  delay(6000);

  Serial.println("Moving Forward");
  roomba.drive(speed,speed);
  delay(4000);

  Serial.println("Turning Left");
  roomba.drive(-speed,speed);
  delay(6000);

  Serial.println("Stopping");
  roomba.drive(0,0);

  Serial.println("Safe Mode");
  roomba.set_mode(roomba_t::SAFE);
}
