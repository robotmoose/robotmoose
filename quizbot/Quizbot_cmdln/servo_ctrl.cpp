#include "servo_ctrl.h"
#include <Servo.h>
#include <Arduino.h>

Servo tilt;
const int tilt_pin=4;
int tilt_forward=38;
int tilt_amount=10;

Servo pan;
const int pan_pin=10;
int pan_forward=90;
int pan_amount=30;

void servo_setup()
{
  tilt.attach(tilt_pin);
  pan.attach(pan_pin);
  face_forward();
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
  for(int times=0;times<2;++times)
  {
    pan.write(pan_forward-pan_amount);
    delay(300);
    pan.write(pan_forward+pan_amount);
    delay(300);
  }
}
	
