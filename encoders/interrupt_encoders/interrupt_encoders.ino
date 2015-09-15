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

Some code borrowed and modified from http://yameb.blogspot.com/2012/11/quadrature-encoders-in-arduino-done.html

*/

#include "digitalWriteFast.h"

#define hall_pin   2
#define led_pin    13
int count; 
volatile bool EncoderSet;
volatile bool EncoderSetPrev;

void setup() 
{
    Serial.begin(9600);
    digitalWrite(hall_pin,HIGH); // pullup resistor is powered
    pinMode(hall_pin,INPUT_PULLUP); // enable pullup and input
    pinMode(led_pin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(hall_pin), IncrementEncoder, CHANGE);
    count = 0;
    EncoderSetPrev = digitalReadFast(hall_pin);
    Serial.println("-------------------------");
    Serial.println("Initializing...");
    Serial.print("Hall Effect Sensor = ");
    Serial.println(EncoderSetPrev);
    Serial.println("-------------------------");
    delay(2000);
}

void loop() 
{
  Serial.print("Interupt Pin: ");
  Serial.println(digitalReadFast(hall_pin));
  
  Serial.print("Encoder Ticks: ");
  Serial.println(count);
  Serial.println("-------------------------");

}

// Interrupt Service Routine to increment encoder counts

void IncrementEncoder()
{
  
  //EncoderSet = digitalReadFast(hall_pin);
  //if (pin_read() != EncoderSetPrev)
  //{
    count = count + 1;
    //EncoderSetPrev = EncoderSet;
    
    if (pin_read())
    {
      digitalWriteFast(led_pin, HIGH);
    }
    
    else digitalWriteFast(led_pin, LOW);
  //}
  
  /*
  else if (pin_read() == EncoderSetPrev)
  {
    count = count;
    EncoderSetPrev = EncoderSet;
    digitalWriteFast(led_pin, LOW);
  }
  */
}

int pin_read()
{
  int pin;
  pin=digitalReadFast(hall_pin);
  
  return pin;
}
  
  
