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
  roomba_sensor_t robot=roomba.get_sensors();

  Serial.print("Floor:  ");

  for(int ii=0;ii<4;++ii)
  {
    Serial.print(robot.floor[ii]);
    Serial.print("\t");
  }

  Serial.println("");
  
  if(robot.floor[2]>50)
  {
    roomba.drive(100,100);
  }
  else
  {
    roomba.drive(-20,-20);
  }
}
