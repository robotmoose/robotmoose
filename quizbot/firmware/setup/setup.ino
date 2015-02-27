#include <Servo.h>

Servo body;
int body_forward=90;
int body_amount=20;

Servo neck;
int neck_forward=30;
int neck_amount=20;

void setup()
{
  body.attach(10);
  neck.attach(9);
}

void loop()
{
  face_forward();
  nod_yes();
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

