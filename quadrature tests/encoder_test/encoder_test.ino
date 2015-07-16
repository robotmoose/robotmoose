// Written By Clayton Auld
// cauld@alaska.edu or clayauld@gmail.com
// Jul 15, 2015

// Arduino test code for Opto-IR Emitter Detector pair for wheel encoder

/*
#define OUT   7  // Logic pin to drive emitter
#define IN    A3  // Analog input pin to read from detector


void setup() 
{
  pinMode(IN, INPUT);
  pinMode(OUT, OUTPUT);
  digitalWrite(OUT, HIGH);
  Serial.begin(115200);
  Serial.println("Setup Done");
}

void loop() 
{
  int X=0;
  X=analogRead(IN);
  Serial.println(X);
  delay(500);
}
*/

/*
  Calibration

 Demonstrates one techinque for calibrating sensor input.  The
 sensor readings during the first five seconds of the sketch
 execution define the minimum and maximum of expected values
 attached to the sensor pin.

 The sensor minumum and maximum initial values may seem backwards.
 Initially, you set the minimum high and listen for anything 
 lower, saving it as the new minumum. Likewise, you set the
 maximum low and listen for anything higher as the new maximum.

 The circuit:
 * Analog sensor (potentiometer will do) attached to analog input 0
 * LED attached from digital pin 9 to ground

 created 29 Oct 2008
 By David A Mellis
 Modified 17 Jun 2009
 By Tom Igoe

 http://arduino.cc/en/Tutorial/Calibration

 */

// These constants won't change:
#define sensorPin    2    // pin that the sensor is attached to
#define ledPin       9    // pin that the LED is attached to

// variables:
int sensorValue = 0;   // the sensor value
int sensorMin = 1023;  // minimum sensor value
int sensorMax = 0;     // maximum sensor value

void setup() {
  Serial.begin(115200);
  // turn on LED to signal the start of the calibration period:
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // calibrate during the first five seconds 
  while (millis() < 5000) {
    sensorValue = analogRead(sensorPin);
    Serial.println(sensorValue);
  
    // record the maximum sensor value
    if (sensorValue > sensorMax) {
      sensorMax = sensorValue;
    }

    // record the minimum sensor value
    if (sensorValue < sensorMin) {
      sensorMin = sensorValue;
    }
  }

  // signal the end of the calibration period
  digitalWrite(13, LOW);
}

void loop() {
  // read the sensor:
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
  
  // apply the calibration to the sensor reading
  sensorValue = map(sensorValue, sensorMin, sensorMax, -255, 255);    // Black = OFF, White = ON  
  //sensorValue = map(sensorValue, sensorMin, sensorMax, 255, -255);  // Black = ON, White = OFF

  // in case the sensor value is outside the range seen during calibration
  sensorValue = constrain(sensorValue, 0, 255);

  // fade the LED using the calibrated value:
  analogWrite(ledPin, sensorValue);
}
