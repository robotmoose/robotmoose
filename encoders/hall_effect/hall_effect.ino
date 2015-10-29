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

#define hall_pin  2
#define led_pin   13

int count1 = 0;
int previousCount = 0;
bool high=false; // Keep track of hall_pin state
bool oldStatus; //hall_pin state for previus loop

void setup()
{
    Serial.begin(9600);
    
    pinMode(hall_pin,INPUT_PULLUP); // enable pullup and input
    //pinMode(led_pin, OUTPUT);
    /*Serial.println("-------------------------");
    Serial.println("Initializing...");
    Serial.println("-------------------------");
    delay(2000);*/
}

void loop() {
      
      oldStatus = high;	
      high=digitalRead(hall_pin);
     
       if(high!=oldStatus)
        {
	  //digitalWrite(led_pin,HIGH); 
          ++count1;    
        }
        
    /*Serial.print("Count1 = ");
    Serial.println(count1);*/
}

