#include "servo_ctrl.h"
#include <Servo.h>

void setup()
{
  Serial.begin(57600);
  servo_setup();

}

void loop()
{
  char x = Serial.read();
  if( x == 'y')
  {
    Serial.println("Yes Received");
    nod_yes();
    face_forward();
    delay(250);
  }
  else if( x == 'n')
  {
    Serial.println("No Received");
    nod_no();
    face_forward();
    delay(250);
  }
}
