#include <Servo.h>

Servo body;
int body_forward=90;
int body_amount=20;

Servo neck;
int neck_forward=30;
int neck_amount=20;

void setup()
{
  Serial.begin(57600);
  Serial.write("Robot starting up!");
  body.attach(10);
  neck.attach(9);
}

void loop()
{
  int key=Serial.read();
  
  if(key=='y')
  {
    nod_yes();
    // add more motions here!
  }
  else if(key=='n')
  {
    shake_no();
    // add more motions here!
  }
  else
  {
    face_forward();
  }
}

void face_forward()
{
  body.write(body_forward+0);
  neck.write(neck_forward+0);
  delay(500);
}

void nod_yes()
{
  body.write(body_forward+0);
  neck.write(neck_forward+neck_amount);
  delay(500);
}

void shake_no()
{
  body.write(body_forward+body_amount);
  neck.write(neck_forward+0);
  delay(500);
}

