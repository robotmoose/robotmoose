/*
Clayton Auld, 
September 11, 2015
clayauld@gmailcom
(Public Domain)

Code to test magnetic hall effect encoders using Melexis US5781 SMD
Pinout is as follows:
    Pin 1 (left):   Vcc to Arduino +5v or +3.3V
    Pin 2 (right):  Vout to Arduino interrupt pin
    Pin 3 (bottom): Gnd to Arduino Gnd
*/

//#include "Arduino.h"
#include <digitalWriteFast.h>

#define hall_pin   0
#define led_pin    13
int count1; int count2; int Total; int flag;

void setup() 
{
    Serial.begin(9600);
    digitalWrite(hall_pin,HIGH); // pullup resistor is powered
    pinMode(hall_pin,INPUT_PULLUP); // enable pullup and input
    pinMode(led_pin, OUTPUT);
    Total = 0;
    count1 = 0;
    count2 = 0;
    flag = digitalRead(hall_pin);
    Serial.println("-------------------------");
    Serial.println("Initializing...");
    Serial.print("Hall Effect Sensor = ");
    Serial.println(flag);
    Serial.println("-------------------------");
    delay(2000);
}

void loop() 
{
  Serial.print("Encoder Ticks: ");
  Serial.println(count);
  Serial.println("-------------------------");

}
