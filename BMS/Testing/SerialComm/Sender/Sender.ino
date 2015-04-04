//Transmits BMS Bytes
#include <stdint.h>
byte percentage = 100;
byte chargeinfoByte = 0xCF;


void setup()
{
  Serial.begin(57600);
}

void loop() {
  Serial.write(percentage);
  Serial.write(chargeinfoByte);
  delay(500);
}
