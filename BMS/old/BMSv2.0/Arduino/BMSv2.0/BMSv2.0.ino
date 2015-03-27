// Written by Clayton Auld
// clayauld@gmail.com
// Jan 14, 2015
// BMS v2.0

#include <Wire.h>
#include <stdlib.h>

// Arduino Uno Pin Parameters

#define A_Cell0    0
#define A_Cell1    1
#define A_Cell2    2
#define D_Cell0    2
#define D_Cell1    3
#define D_Cell2    4

float Cell0;
float Cell1;
float Cell2;

void setup() 
{
  Serial.begin(9600);
  
  // Arduino Uno Digital Pin Settings
  pinMode(D_Cell0, OUTPUT);
  pinMode(D_Cell1, OUTPUT);
  pinMode(D_Cell2, OUTPUT);
}

void ReadVoltage()
{
  int sensor0 = analogRead(A_Cell0);
  //Serial.println(sensor0);
  Cell0 = sensor0 * (5.0/1023.0);
  int sensor1 = analogRead(A_Cell1);
  //Serial.println(sensor1);
  Cell1 = sensor1 * (5.0/1023.0);
  Cell1 = Cell1 - Cell0;
  int sensor2 = analogRead(A_Cell2);
  //Serial.println(sensor2);
  Cell2 = sensor2 *2* (5.0/1023.0);
  Cell2 = Cell2 - Cell1 - Cell0;
}
  

void PrintVoltage()
{
  Serial.println(Cell0, 3);
  Serial.println(Cell1, 3);
  Serial.println(Cell2, 3);
  Serial.println("--------------");
}

void Balance()
{
  digitalWrite(D_Cell0, HIGH);
  digitalWrite(D_Cell1, HIGH);
  digitalWrite(D_Cell2, HIGH);
  delay(500);
  digitalWrite(D_Cell0, LOW);
  delay(500);
  digitalWrite(D_Cell0, HIGH);
  digitalWrite(D_Cell1, LOW);
  delay(500);
  digitalWrite(D_Cell1, HIGH);
  digitalWrite(D_Cell2, LOW);
  delay(500);
  digitalWrite(D_Cell0, LOW);
  digitalWrite(D_Cell1, LOW);
  digitalWrite(D_Cell2, HIGH);
  delay(500);
  digitalWrite(D_Cell0, HIGH);
  digitalWrite(D_Cell1, LOW);
  digitalWrite(D_Cell2, LOW);
  delay(500);
  digitalWrite(D_Cell0, LOW);
  digitalWrite(D_Cell1, HIGH);
  digitalWrite(D_Cell2, LOW);
  delay(500);
  digitalWrite(D_Cell0, HIGH);
  digitalWrite(D_Cell1, HIGH);
  digitalWrite(D_Cell2, HIGH);
}

void loop() 
{
  ReadVoltage();
  Balance();
  PrintVoltage();
  delay(500);

}
