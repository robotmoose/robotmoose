//Reads data from serial
//Transmits BMS Bytes
#include <stdint.h>

const int numbytes = 2;
byte data[numbytes];
uint8_t percentage;
uint8_t chargeinfoByte;

void setup() {
  Serial.begin(9600);
  Serial1.begin(57600);
}

void loop() {
  for(int i = 0; i < numbytes; i++){
    data[i] = Serial1.read();
  }
  if(data[0] & 0xF0 == 0xC0){
    chargeinfoByte = data[0];
    percentage = data[1];
  }
  else if(data[1] & 0xF0 == 0xC0){
    chargeinfoByte = data[1];
    percentage = data[0];
  }
  else{
    //Error sending or reading
  }
  Serial.println(data[0]);
  Serial.println(data[1]);
  delay(1200);
}
