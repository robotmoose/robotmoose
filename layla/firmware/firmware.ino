/**
 * Aurora Robotics "Autonomy" firmware
 * For Arduino Uno, hooked to LAYLA telepresence platform.
 */
#include "robot.h" /* classes shared with PC, and across network */
#include "serial_packet.h" /* CYBER-Alaska packetized serial comms */

#include <SoftwareSerial.h>
#include "SoftwareServo.h"
//#include <Servo.h>

void LEDdemo();
void sendMotor(int motorSide,int power);
void low_latency_ops();
void read_sensors(void);
void send_servos(void) ;
void send_motors(void);
void handle_packet(A_packet_formatter<HardwareSerial> &pkt,const A_packet &p);
void LEDdemo();

struct leds
{
  leds(int b, int r, int g): blue(b), red(r), green(g)
  {
  }
  int blue;
  int red;
  int green;
};

struct ramp 
{
  ramp(leds a,unsigned long s): active(a), slowness(s), lastrun(0), brightness(0), up(true)
  {
  }
  void run();
  bool up;
  unsigned long lastrun;
  leds active;
  int brightness;
  unsigned long slowness;
};


/** This class manages communication via an A_packet_formatter,
 including timeouts. */
class CommunicationChannel {
public:
  HardwareSerial &backend;
  A_packet_formatter<HardwareSerial> pkt; // packet formatter
  bool is_connected; // 1 if we're recently connected; 0 if no response
  unsigned long last_read; // millis() the last time we got data back
  unsigned long next_send; // millis() the next time we should send off data

  CommunicationChannel(HardwareSerial &new_backend) :
  backend(new_backend), pkt(backend) 
  {
    is_connected=0;
    last_read=0;
    next_send=0;
  }

  bool read_packet(unsigned long milli, A_packet &p) {
    p.valid=0;
    if (backend.available()) {
      while (-1==pkt.read_packet(p)) { 
        low_latency_ops(); /* while reading packet */
      }
      if (p.valid) {
        last_read=milli; 
        next_send=milli+500;
        is_connected=true; // got valid packet
        return true;
      }
    }
    if (milli>next_send) { // read timeout
      next_send=milli+500;
      pkt.reset();
      pkt.write_packet(0,0,0); // send heartbeat ping packet
      is_connected=false;
      digitalWrite(13,LOW);
    }
    return false;
  }
};

//-------------
SoftwareSerial saberSerial(9,8); // RX (not used), TX

leds ledpins(11,10,9);  // which pins are used for RGB led's
ramp colors(leds(1,0,0),10);  //which colors to ramp though (led(RED,GREEN,Blue),Speed)

SoftwareServo servo1;  //servo objects for pins look at setup servo.attach
SoftwareServo servo2;
SoftwareServo servo3;


// All PC commands go via this (onboard USB) port
HardwareSerial &PCport=Serial; // direct PC
CommunicationChannel PC(PCport);
// Robot's current state:
robot_current robot;

unsigned long next_micro_send=0;
//--------------




/***************** Robot Control Logic ****************/

void setup()
{
  Serial.begin(9600); // Control connection to PC
  saberSerial.begin(9600); // sabertooth motor controller
 
  //servo signal pins
  servo1.attach(3);
  servo2.attach(5);
  servo3.attach(6);
  
  //set servos to known state in case of power cycle
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  
  //set led pin's up
  pinMode(ledpins.blue, OUTPUT);
  pinMode(ledpins.red, OUTPUT);
  pinMode(ledpins.green, OUTPUT);
  analogWrite(ledpins.blue,0); // turn them off
  analogWrite(ledpins.red,0);
  analogWrite(ledpins.green,0);
  
  // Our ONE debug LED!
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);

}


void loop()
{
  unsigned long micro=micros();
  unsigned long milli=micro>>10; // approximately == milliseconds

  A_packet p;
  if (PC.read_packet(milli,p)) handle_packet(PC.pkt,p);
  if (!(PC.is_connected)) robot.power.stop(); // disconnected?

  if (micro>=next_micro_send) 
  { // Send commands to motors
    send_motors();
   // send_servos();
    next_micro_send=micro+25*1024; // send_motors takes 20ms
  }
  low_latency_ops();
  if(robot.led.ledon)
  {
    if(robot.led.demo)
      LEDdemo();
  }
  else
   {
    analogWrite(ledpins.blue,0);
    analogWrite(ledpins.red,0);
    analogWrite(ledpins.green,0);
   }
     
  //SoftwareServo::refresh();
}


// Call this function frequently--it's for minimum-latency operations
void low_latency_ops() {
  if(robot.led.ledon)
    if(robot.led.demo)
       colors.run();
}

void sendMotor(int motorSide,int power) {
  if (power<1) power=1;
  if (power>126) power=126;
  if (motorSide) power+=128;
  saberSerial.write((unsigned char)power);
}

// Read all robot sensors into robot.sensor
void read_sensors(void) {
  /* no sensors on LAYLA, so far! */
  low_latency_ops();
}

// Sends servo values 0-180
// command is quick and nonblocking
void send_servos(void) 
{
  servo1.write((robot.power.front*180)/127);  // front is 0-127 and servo range is 0-180
  servo2.write((robot.power.mine*180)/127);  // servo2
  servo3.write((robot.power.dump*180)/127);  //servo3
}

// Send current motor values to sabertooths.
//   These 5 writes take a total of 20ms *if* the serial line is busy
void send_motors(void) {
  sendMotor(0,robot.power.left);
  low_latency_ops();
  sendMotor(1,robot.power.right);
  low_latency_ops();
}

// Structured communication with PC:
void handle_packet(A_packet_formatter<HardwareSerial> &pkt,const A_packet &p)
{
  if (p.command==0x7) { // motor power commands
    low_latency_ops();
    if (!p.get(robot.power)) { // error
      pkt.write_packet(0xE,0,0);
    }
    else 
    { // got power request successfully: read and send sensors
      low_latency_ops(); /* while reading sensors */
      read_sensors();
      low_latency_ops();
      pkt.write_packet(0x3,sizeof(robot.sensor),&robot.sensor);
      robot.sensor.latency=0; // reset latency metric
      low_latency_ops();

      static bool blink=0;
      digitalWrite(13,!blink); // good input received: blink!
      blink=!blink;
    }
  }
  else if (p.command==0) { // ping request
    pkt.write_packet(0,p.length,p.data); // ping reply
  }
  else if (p.command==0xC) // led command
  { // Led change
	 if (!p.get(robot.led)) 
	 { // error
            pkt.write_packet(0xE,0,0);
         }
	else // packet is good
	{
         analogWrite(ledpins.blue,robot.led.blue); // turn them to what we got
         analogWrite(ledpins.red,robot.led.red);
         analogWrite(ledpins.green,robot.led.green);
	}
  }
}

//lights demo code (needs cleaned)
void LEDdemo()
{

static int count=0;
static int countspace=5000;
static unsigned long lastcount=0;
  if(lastcount+countspace<millis())
  {
    lastcount=millis();
    count++;
  
    if(count>7)
      count=0;
    
    if(count==0)
    {
      colors.active.blue=1;
      colors.active.red=0;
      colors.active.green=0;
      analogWrite(ledpins.blue,0);
      analogWrite(ledpins.red,0);
      analogWrite(ledpins.green,0);
    }
    if(count==1)
    {
      colors.active.blue=0;
      colors.active.red=1;
      colors.active.green=0;
      analogWrite(ledpins.blue,0);
      analogWrite(ledpins.red,0);
      analogWrite(ledpins.green,0);
    }
    if(count==3)
    {
      colors.active.blue=0;
      colors.active.red=0;
      colors.active.green=1;
      analogWrite(ledpins.blue,0);
      analogWrite(ledpins.red,0);
      analogWrite(ledpins.green,0);
    }
    if(count==4)
    {
      colors.active.blue=0;
      colors.active.red=1;
      colors.active.green=1;
      analogWrite(ledpins.blue,0);
      analogWrite(ledpins.red,0);
      analogWrite(ledpins.green,0);
    }
    if(count==5)
    {
      colors.active.blue=1;
      colors.active.red=1;
      colors.active.green=0;
      analogWrite(ledpins.blue,0);
      analogWrite(ledpins.red,0);
      analogWrite(ledpins.green,0);
    }
    if(count==6)
    {
      colors.active.blue=1;
      colors.active.red=0;
      colors.active.green=1;
      analogWrite(ledpins.blue,0);
      analogWrite(ledpins.red,0);
      analogWrite(ledpins.green,0);
    }
    if(count==7)
    {
      colors.active.blue=1;
      colors.active.red=1;
      colors.active.green=1;
      analogWrite(ledpins.blue,0);
      analogWrite(ledpins.red,0);
      analogWrite(ledpins.green,0);
    }
  }
}


void ramp::run()
{
  if(lastrun+slowness<millis())
  {
    if(up)
      brightness++;
    else
      brightness--;
    lastrun=millis();
    if(brightness<0)
    {
      brightness=0;
      up=true;
    }
    if(brightness>255)
    {
      brightness=255;
      up=false;
    }
    if(active.blue==1)
      analogWrite(ledpins.blue, brightness);
    if(active.red==1)
      analogWrite(ledpins.red, brightness);
    if(active.green==1)
      analogWrite(ledpins.green, brightness);
  }
}
