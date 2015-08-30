/*
  Trivial code to dump the output from a SparkFun hall effect sensor,
  a Unisonic U18202.

  Pinout, with part face up and headed right
    vcc, to arduino 5v or 3.3v
    gnd, to arduino gnd
    out, to arduino digital pin

  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-07-17 (Public Domain)

  This version is coded for the Particle Photon.

  Clayton Auld, clayauld@gmail.com, 2015-07-24

*/

#define hall_pin  6
#define led_pin   13
int count1; int count2; int Total; int flag;

void setup()
{
    Serial.begin(9600);
    digitalWrite(hall_pin,HIGH); // pullup resistor is powered
    pinMode(hall_pin,INPUT_PULLUP); // enable pullup and input
    pinMode(led_pin, OUTPUT);
    Total = 0;
    count1 = 0;
    count2 = 0;
    flag = digitalRead(hall_pin);
    Serial.println("-------------------------");
    Serial.println("Initializing...");
    Serial.print("Hall Effect Sensor = ");
    Serial.println(flag);
    Serial.println("-------------------------");
    delay(2000);
    //pinMode(relay, OUTPUT);
}

void loop() {
  
    if (digitalRead(hall_pin)) {
      //Serial.println("1");
      digitalWrite(led_pin,HIGH);
      if (flag == 0) {
          count1 = count1 + 1;
          flag = 1;
      }
      
      //Serial.print("flag = ");
      //Serial.println(flag);
    }
    
    else {
      //Serial.println("0");
      digitalWrite(led_pin,LOW);
      if (flag == 1) {
          count2 = count2 + 1;
          flag = 0;
      }
      
      //Serial.print("flag = ");
      //Serial.println(flag);
    }
    Total = count1 + count2;
    
    //Serial.print("Count1 = ");
    //Serial.println(count1);
    //Serial.print("Count2 = ");
    //Serial.println(count2);
    Serial.print("Total = ");
    Serial.println(Total);
}
