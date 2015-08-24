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
#define led_pin   13
int count; int flag;

void setup()
{
    Serial.begin(9600);
    digitalWrite(hall_pin,HIGH); // pullup resistor is powered
    pinMode(hall_pin,INPUT_PULLUP); // enable pullup and input
    pinMode(led_pin, OUTPUT);
    count = 0;
    flag = 0;

    //pinMode(relay, OUTPUT);
}

void loop() {

    if (digitalRead(hall_pin)) {
      //Serial.println("1");
      digitalWrite(led_pin,LOW);
      flag = 0;
      //digitalWrite(relay,LOW);
      //Serial.println("No magnet detected!");
      //Serial.println(analogRead(A1));
    }
    else {
      //Serial.println("0");
      digitalWrite(led_pin,HIGH);
      if (flag == 0) {
          count = count + 1;
          flag = 1;
      }
    }

    Serial.print("Count = ");
    Serial.println(count);
}
