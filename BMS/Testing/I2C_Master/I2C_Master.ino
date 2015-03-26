// Wire Master Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>
#include <stdlib.h>

//byte x=0;
//float Cell_0;
//float Cell_1;
//float Cell_2;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
}

void readI2C()
{
    byte c;
    Wire.requestFrom(2, 1);    // request 2 bytes from slave device #2
    while(Wire.available())    // slave may send less than requested
    {
        c = Wire.read(); // receive a byte as character
        Serial.print(c);   // print the character
        Serial.println("");
    }
    //Cell0=atof(c);
}

void loop()
{
  readI2C();
  
 Serial.println("");
// 
// Serial.print("Cell 0: ");
// Serial.println(Cell_0,4);
// Serial.print("Cell 1: ");
// Serial.println(Cell_1,4);
// Serial.print("Cell 2: ");
// Serial.println(Cell_2,4);
 Serial.println("---------------------------");
}
