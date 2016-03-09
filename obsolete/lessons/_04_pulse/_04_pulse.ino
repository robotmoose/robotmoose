void setup() {
  // put your setup code here, to run once:
  pinMode(3,OUTPUT);     //Setup pin 3 to "output" a signal.
  pinMode(4,OUTPUT);     //Setup pin 4 to "output" a signal.
  digitalWrite(3,LOW);   //Set pin 3 to LOW (also called ground).
  digitalWrite(4,LOW);   //Set pin 4 to LOW (also called ground).
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int ii=0;ii<=255;++ii) {
    analogWrite(3,ii);   //Write the value "ii" to the LED
    delay(5);            //Wait a bit so the human eye.
  }
  for(int ii=255;ii>=0;--ii) {
    analogWrite(3,ii);   //Write the value "ii" to the LED
    delay(5);            //Wait a bit so the human eye.
  }
}
