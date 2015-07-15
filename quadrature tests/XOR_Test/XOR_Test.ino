// Written By Clayton Auld
// cauld@alaska.edu or clayauld@gmail.com
// Jul 14, 2015

// Arduino test code for XOR gate made from TI SN74LS00N chip.

#define IN1    5  // Input 1 for XOR
#define IN2    6  // Input 2 for XOR
#define OUT    7  // OUTPUT for XOR

void setup() 
{
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(OUT, INPUT);
  
  Serial.begin(115200);
  Serial.println("Setup Done");
}

void loop() 
{
  int Q=0;
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  Q=digitalRead(OUT);
  Serial.println("Input to XOR Gate: 00");
  Serial.print("Output: ");
  Serial.println(Q);
  
  delay(1000);
  
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  Q=digitalRead(OUT);
  Serial.println("Input to XOR Gate: 01");
  Serial.print("Output: ");
  Serial.println(Q);
  
  delay(1000);
  
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  Q=digitalRead(OUT);
  Serial.println("Input to XOR Gate: 11");
  Serial.print("Output: ");
  Serial.println(Q);
  
  delay(1000);
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  Q=digitalRead(OUT);
  Serial.println("Input to XOR Gate: 10");
  Serial.print("Output: ");
  Serial.println(Q);
  
  delay(1000);
}
