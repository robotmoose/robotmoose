#include <Servo.h>       //This tells us how to use a servo

Servo myservo;           //Create a servo object

void setup() {
  // put your setup code here, to run once:
  myservo.attach(9);     //Attach servo to pin 9
  myservo.write(180);    //Move servo to 180 degrees
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int ii=180;ii>=0;--ii) {
    myservo.write(ii);   //Move servo to ii degrees
    delay(20);           //Wait a bit so the human eye.
  }
}
