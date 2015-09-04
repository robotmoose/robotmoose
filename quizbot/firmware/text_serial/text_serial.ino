#include <Servo.h>

Servo tilt;
int tilt_pin=9;
int tilt_forward=38;
int tilt_amount=10;

Servo pan;
int pan_pin=10;
int pan_forward=90;
int pan_amount=30;

char* question="  What is 9*8?";
char* answers[3]={"    1  72","    2  81","    3  17"};
int correct_answer=1;

void setup()
{
  Serial.begin(57600);
  tilt.attach(tilt_pin);
  pan.attach(pan_pin);
  face_forward();
  delay(500);

  Serial.println("Quizbot!");
  Serial.println(question);

  for(int ii=0;ii<3;++ii)
    Serial.println(answers[ii]);
}

void loop()
{
  int answer=Serial.parseInt();

  if(answer>0)
  {
    if(answer==correct_answer)
    {
      Serial.println("  Correct! :)");
      nod_yes();
    }
    else
    {
      Serial.println("  Incorrect! :(");
      nod_no();
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
