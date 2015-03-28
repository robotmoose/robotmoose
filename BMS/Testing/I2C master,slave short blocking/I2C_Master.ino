// Wire Master Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>
#include <stdlib.h>
long int time= 0;
void setup()
{
  Wire.begin(2);        // join i2c bus (address optional for master)
  Wire.onReceive(readLine);
  Serial.begin(57600);  // start serial for output
}

void readLine(int number)
{
  long int delay1=micros();
  Serial.print("delay of reading input=");
    Serial.println(delay1-time);
}

void readI2C()
{
   time=micros();
    if(2>=Wire.available())    // slave may send less than requested
    while(Wire.available())
    {
       time=micros();
       byte x = Wire.read(); // receive a byte as character
        time=micros();
       Serial.print(x);   // print the character
        time=micros();
       Serial.println("");
        time=micros();
    }
}



void loop()
{
 time=micros();
  readI2C();
  time=micros();
}
