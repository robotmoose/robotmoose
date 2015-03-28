/*
Reading 9DOF sensor stick from Sparkfun 
 http://www.sparkfun.com/products/10724

Arduino Uno R3: 
  VCC 5V
  gnd 0v
  SDA A4
  SCL A5

Original by DARPA Team Mojavaton
http://playground.arduino.cc//Main/SEN-10724

Slight cleanup by Dr. Orion Lawlor, lawlor@alaska.edu, 2014-11-04 (Public Domain)
*/
#include <Wire.h>

#define GYROADDR 0x68
#define COMPASSADDR 0x1e
#define ACCELADDR 0x53

union XYZBuffer {
  struct {
    short x,y,z;
  } value;
  byte buff[6];
};

void changeEndian(union XYZBuffer *xyz) {
  for (int i=0;i<6;i+=2) {
    byte t=xyz->buff[i];
    xyz->buff[i]=xyz->buff[i+1];
    xyz->buff[i+1]=t;
  }
}

// Generically useful reading into a union type
void readXYZ(int device,union XYZBuffer *xyz) {     
  Wire.requestFrom(device, 6);      
  long start=millis();
  while (!Wire.available() && (millis()-start)<100);
  if (millis()-start<100) {
    for (int i=0;i<6;i++)
      xyz->buff[i]=Wire.read();
  }
}

void setupAccel(int device) {
  // Check ID to see if we are communicating
  Wire.beginTransmission(device);
  Wire.write(0x00); // One Reading
  Wire.endTransmission(); 
  Wire.requestFrom(device,1);
  while (!Wire.available());  
  byte ch=Wire.read();
  Serial.print("Accel id is 0x");
  Serial.println(ch,HEX);
  // Should output E5

  // https://www.sparkfun.com/datasheets/Sensors/Accelerometer/ADXL345.pdf
  // Page 16
  Wire.beginTransmission(device);
  Wire.write(0x2d);  // POWER_CTL 
  Wire.write(0x08);
  Wire.endTransmission();
  Wire.beginTransmission(device);
  Wire.write(0x38); // FIFO control
  Wire.write(0x84); // 4-entry fifo
  Wire.endTransmission();

}
void readAccel(int device,union XYZBuffer *xyz) {
  Wire.beginTransmission(device);
  Wire.write(0x32); // One Reading: data format XYZ
  Wire.endTransmission(); 
  readXYZ(device,xyz);
  xyz->value.x=-xyz->value.x; // flip x, to agree with board and be right-handed coords
}

void setupCompass(int device) {
  // Check ID to see if we are communicating
  Serial.print("Compass id is ");
  Wire.beginTransmission(device);
  Wire.write(10); // One Reading
  Wire.endTransmission(); 
  Wire.requestFrom(device,2); 
  while (!Wire.available());
  char ch=Wire.read();
  Serial.print(ch);   
  ch=Wire.read();
  Serial.println(ch);
  // Should output H4  

// Page 18
// at http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/Magneto/HMC5883L-FDS.pdf
  Wire.beginTransmission(device);
  Wire.write(0x00); Wire.write(0x70);
  Wire.endTransmission();
  Wire.beginTransmission(device);
  Wire.write(0x01); Wire.write(0xA0);
  Wire.endTransmission();
  Wire.beginTransmission(device);
  Wire.write(0x02); Wire.write(0x00); //  Reading
  Wire.endTransmission(); 
  delay(6);
}
void readCompass(int device,union XYZBuffer *xyz) {
  readXYZ(device,xyz);
  changeEndian(xyz);
  Wire.beginTransmission(device);
  Wire.write(0x03);
  Wire.endTransmission(); 
}

void setupGyro(int device) {
  // Check ID to see if we are communicating
  Wire.beginTransmission(device);
  Wire.write(0x00); // One Reading
  Wire.endTransmission(); 
  Wire.requestFrom(device,1);
  while (!Wire.available());  
  byte ch=Wire.read();
  Serial.print("Gyro id is 0x");
  Serial.println(ch,HEX);  
  // Should output 69
}
void readGyro(int device,union XYZBuffer *xyz) {
  // https://www.sparkfun.com/datasheets/Sensors/Gyro/PS-ITG-3200-00-01.4.pdf
  // page 20
  Wire.beginTransmission(device);
  Wire.write(0x1d);
  Wire.endTransmission(); 
  readXYZ(device,xyz);
  changeEndian(xyz);  
}

void output(union XYZBuffer xyz) {
  Serial.print(xyz.value.x);
  Serial.print('	');
  Serial.print(xyz.value.y);
  Serial.print('	');
  Serial.print(xyz.value.z);
}

void setup()
{
  Serial.begin(57600);  // start serial for output
  Wire.begin();        // join i2c bus (address optional for master)
  setupCompass(COMPASSADDR);
  setupAccel(ACCELADDR);
  setupGyro(GYROADDR);
}

void loop()
{
  union XYZBuffer compass,gyro,accel;
  readAccel(ACCELADDR,&accel);
  readCompass(COMPASSADDR,&compass);
  readGyro(GYROADDR,&gyro);
  Serial.print("A	");
  output(accel);
  Serial.print("	G	");  
  output(gyro);
  Serial.print("	C	");
  output(compass);
  Serial.println();
}

