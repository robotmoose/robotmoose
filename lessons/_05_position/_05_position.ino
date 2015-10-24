#include <Servo.h>       //This tells us how to use a servo

Servo myservo;           //Create a servo object

void setup() {
  // put your setup code here, to run once:
  myservo.attach(9);     //Attach servo to pin 9
  myservo.write(90);     //Move servo to 90 degrees
}

void loop() {
  // put your main code here, to run repeatedly:
}
