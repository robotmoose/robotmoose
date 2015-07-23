/**
  Trivial code to dump the output from a SparkFun hall effect sensor,
  a Unisonic U18202.
  
  Pinout, with part face up and headed right
    vcc, to arduino 5v
    gnd, to arduino gnd
    out, to arduino digital pin
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-07-17 (Public Domain)
*/

#define hall_pin  6
#define led_pin   9

void setup() {
    Serial.begin(57600);
    digitalWrite(hall_pin,HIGH); // pullup resistor is powered
    pinMode(hall_pin,INPUT_PULLUP); // enable pullup and input
}

void loop() {
    if (digitalRead(hall_pin)) {
      Serial.println("1");
      analogWrite(led_pin,800);
      //Serial.println(analogRead(A1));
    }
    else {
      Serial.println("0");
      analogWrite(led_pin,0);
      //Serial.println(analogRead(A1));
    //delay(50);
    }
}

