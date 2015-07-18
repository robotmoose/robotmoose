/**
  Trivial code to dump the output from a SparkFun hall effect sensor,
  a Unisonic U18202.
  
  Pinout, with part face up and headed right
    vcc, to arduino 5v
    gnd, to arduino gnd
    out, to arduino digital pin
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-07-17 (Public Domain)
*/

int hall_pin=6;

void setup() {
    Serial.begin(57600);
    digitalWrite(hall_pin,HIGH); // pullup resistor is powered
    pinMode(hall_pin,INPUT_PULLUP); // enable pullup and input
}
void loop() {
    if (digitalRead(hall_pin)) Serial.println("1");
    else Serial.println("0");
    delay(50);
}

