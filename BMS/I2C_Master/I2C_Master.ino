// Wire Master Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>
#include <stdlib.h>

byte x=0;
float Cell0;
float Cell1;
float Cell2;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
}

void readI2C()
{
  if (x==0)
  {
    Wire.requestFrom(2, 6);    // request 6 bytes from slave device #2
    int i=0;
    char c[6];
    String string;
    while(Wire.available())    // slave may send less than requested
    {
        c[i] = Wire.read(); // receive a byte as character
        // Serial.print(Cell[i]);   // print the character
        i++;
    }
    Cell0=atof(c);
  }
  
  else if (x==1)
  {
    Wire.requestFrom(2, 6);    // request 6 bytes from slave device #2
    int i=0;
    char c[6];
    String string;
    while(Wire.available())    // slave may send less than requested
    {
        c[i] = Wire.read(); // receive a byte as character
        // Serial.print(Cell[i]);   // print the character
        i++;
    }
    Cell1=atof(c);
  }
  
  else if (x==2)
  {
    Wire.requestFrom(2, 6);    // request 6 bytes from slave device #2
    int i=0;
    char c[6];
    String string;
    while(Wire.available())    // slave may send less than requested
    {
        c[i] = Wire.read(); // receive a byte as character
        // Serial.print(Cell[i]);   // print the character
        i++;
    }
    Cell2=atof(c);
  }
}
 
void sendI2C()
{
  //Serial.println(x);
  Wire.beginTransmission(2);
  Wire.write(x);
  Wire.endTransmission();
}

void loop()
{
  sendI2C();
  readI2C();

 Serial.println("");
 x++;
 if (x>2) x=0;
 delay(2000);
 
 Serial.print("Cell 0: ");
 Serial.println(Cell0,3);
 Serial.print("Cell 1: ");
 Serial.println(Cell1,3);
 Serial.print("Cell 2: ");
 Serial.println(Cell2,3);
 Serial.println("---------------------------");
}


/*void loop()
{
  Wire.requestFrom(2, 6);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }
  Serial.println("");

  delay(1200);
}*/
