#include <Servo.h>

Servo tilt;
int tilt_pin=4;
int tilt_forward=38;
int tilt_amount=10;

Servo pan;
int pan_pin=10;
int pan_forward=90;
int pan_amount=30;

void setup()
{
  Serial.begin(57600);
  tilt.attach(tilt_pin);
  pan.attach(pan_pin);
  face_forward();
  delay(500);
}

void loop()
{
  char temp;

  while(Serial.available()>0&&Serial.readBytes(&temp,1)==1)
  {
    if(temp=='n')
    {
      nod_no();
    }
    else if(temp=='y')
    {
      nod_yes();
    }

    face_forward();
    delay(500);
  }
}

void face_forward()
{
  tilt.write(tilt_forward);
  pan.write(pan_forward);
}

void nod_yes()
{
  for(int times=0;times<3;++times)
  {
    tilt.write(tilt_forward-tilt_amount);
    delay(300);
    tilt.write(tilt_forward+tilt_amount);
    delay(300);
  }
}

void nod_no()
{
  for(int times=0;times<3;++times)
  {
    pan.write(pan_forward-pan_amount);
    delay(300);
    pan.write(pan_forward+pan_amount);
    delay(300);
  }
}
