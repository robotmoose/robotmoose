/**
 * Aurora Robotics "Autonomy" firmware
 * For Arduino Uno, hooked to LAYLA telepresence platform.
 */
#include "robot.h" /* classes shared with PC, and across network */
#include "serial_packet.h" /* CYBER-Alaska packetized serial comms */

#include <SoftwareSerial.h>
#include "SoftwareServo.h"
#include <Servo.h>

void low_latency_ops();  // fast operations

void sendMotor(int16_t motorSide,int16_t power); //sends power level of motor to sabortooth
void send_servos(void) ;  //requests servos to be at angle however, (SoftwareServo::refresh()) set servor to that angle
void send_motors(void);  //sets all motors power levels

void handle_packet(A_packet_formatter<HardwareSerial> &pkt,const A_packet &p);  //reads serial packets, and sometimes read/sends sensors

void read_sensors(void);  //get sensor values and store in robot.sensors (preportory for sending)
void send_sensors(void); //sends the sensor values to PC
void u_received();  //interupt function for

void send_leds(void) ;   //sets RGB led color
void LEDdemo();  //auto changing led colors

#define LAYLA_UNO


#ifdef LAYLA_UNO
  #define PIN_LED_RED 11
  #define PIN_LED_GREEN 10
  #define PIN_LED_BLUE 9
  #define PIN_SABER_RX 9
  #define PIN_SABER_TX 8
  #define PIN_USOUND1_TRG 12
  #define PIN_USOUND2_TRG 7
  #define PIN_USOUND3_TRG 4
  #define PIN_USOUND4_TRG A5
  #define PIN_USOUND5_TRG A4
  #define PIN_SERVO1 3
  #define PIN_SERVO2 5
  #define PIN_SERVO3 6
  #define PIN_DEBUG 13
  #define PIN_USOUND_READ 2
  // All PC commands go via this (onboard USB) port
  HardwareSerial &PCport=Serial; // direct PC
  #define INTTERUPT_USOUND 0
  SoftwareSerial saberSerial(PIN_SABER_RX,PIN_SABER_TX); // RX (not used), TX

  SoftwareServo servo1;  //servo objects for pins look at setup servo.attach
  SoftwareServo servo2;
  SoftwareServo servo3;
#else
  #define PIN_LED_RED 11
  #define PIN_LED_GREEN 10
  #define PIN_LED_BLUE 9
  //#define PIN_SABER_RX 19
  //#define PIN_SABER_TX 18
  HardwareSerial &saberSerial=Serial1; // RX (not used), TX
  #define PIN_USOUND1_TRG 12
  #define PIN_USOUND2_TRG 7
  #define PIN_USOUND3_TRG 4
  #define PIN_USOUND4_TRG A5
  #define PIN_USOUND5_TRG A4
  #define PIN_SERVO1 3
  #define PIN_SERVO2 5
  #define PIN_SERVO3 6
  #define PIN_DEBUG 13
  #define PIN_USOUND_READ 2
  // All PC commands go via this (onboard USB) port
  HardwareSerial &PCport=Serial; // direct PC
  #define INTTERUPT_USOUND 0
  Servo servo1;  //servo objects for pins look at setup servo.attach
  Servo servo2;
  Servo servo3;
#endif

struct leds
{
  leds(int16_t b, int16_t r, int16_t g): blue(b), red(r), green(g)
  {
  }
  int16_t blue;
  int16_t red;
  int16_t green;
};

struct usound
{
  usound(): pulse_width(0), state(0), start(0),wait(false),current(1)
  {}
  uint32_t pulse_width;
  int16_t state; //0 low start trig, 1 high trig, 2 low wait for echo
  uint32_t start;
  int16_t wait; //1 we are, 0 we are not waiting
  int16_t current; // current sensor 1-5
};

struct ramp
{
  ramp(leds a,uint32_t s): active(a), slowness(s), lastrun(0), brightness(0), up(true)
  {
  }
  void run();
  bool up;
  uint32_t lastrun;
  leds active;
  int16_t brightness;
  uint32_t slowness;
};


/** This class manages communication via an A_packet_formatter,
 including timeouts. */
class CommunicationChannel {
public:
  HardwareSerial &backend;
  A_packet_formatter<HardwareSerial> pkt; // packet formatter
  bool is_connected; // 1 if we're recently connected; 0 if no response
  uint32_t last_read; // millis() the last time we got data back
  uint32_t next_send; // millis() the next time we should send off data

  CommunicationChannel(HardwareSerial &new_backend) :
  backend(new_backend), pkt(backend)
  {
    is_connected=0;
    last_read=0;
    next_send=0;
  }

  bool read_packet(uint32_t milli, A_packet &p) {
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
      digitalWrite(PIN_DEBUG,LOW);
    }
    return false;
  }
};



//------------- globals

leds ledpins(PIN_LED_RED,PIN_LED_GREEN,PIN_LED_BLUE);  // which pins are used for RGB led's
ramp colors(leds(1,0,0),10);  //which colors to ramp though (led(RED,GREEN,Blue),Speed)

// All PC commands go via this (onboard USB) port
CommunicationChannel PC(PCport);
// Robot's current state:
robot_current robot;

uint32_t next_micro_send=0;
usound ultraSound;
//--------------




/***************** Robot Control Logic ****************/

void setup()
{
  Serial.begin(9600); // Control connection to PC
  saberSerial.begin(9600); // sabertooth motor controller

  //servo signal pins
  servo1.attach(PIN_SERVO1);
  servo2.attach(PIN_SERVO2);
  servo3.attach(PIN_SERVO3);

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
  pinMode(PIN_DEBUG,OUTPUT);
  digitalWrite(PIN_DEBUG,LOW);

  //ultra sound setup
  pinMode(PIN_USOUND1_TRG, OUTPUT);
    digitalWrite(PIN_USOUND1_TRG,LOW);
  pinMode(PIN_USOUND2_TRG, OUTPUT);
    digitalWrite(PIN_USOUND2_TRG,LOW);
  pinMode(PIN_USOUND3_TRG, OUTPUT);
    digitalWrite(PIN_USOUND3_TRG,LOW);
  pinMode(PIN_USOUND4_TRG, OUTPUT);
    digitalWrite(PIN_USOUND4_TRG,LOW);
  pinMode(PIN_USOUND5_TRG, OUTPUT);
    digitalWrite(PIN_USOUND5_TRG,LOW);
  pinMode(PIN_USOUND_READ,INPUT);
  attachInterrupt(INTTERUPT_USOUND, u_received, CHANGE);

}


void loop()
{
  static uint32_t test=0;
  uint32_t micro=micros();
  uint32_t milli=micro>>10; // approximately == milliseconds

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
  send_leds();  // only needs to be called once every 5sec for demo

  //SoftwareServo::refresh();  // can be slow and blocking (no interupts)
}


// Call this function frequently--it's for minimum-latency operations
void low_latency_ops() {
  if(robot.led.ledon)
    if(robot.led.demo)
       colors.run();
  read_sensors();
}




// motor and servo stuff------------

void sendMotor(int16_t motorSide,int16_t power) {
  if (power<1) power=1;
  if (power>126) power=126;
  if (motorSide) power+=128;
  saberSerial.write((uint8_t)power);
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



//serial data--------------

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
      read_sensors();
      send_sensors();
      low_latency_ops();

      static bool blink=0;
      digitalWrite(PIN_DEBUG,!blink); // good input received: blink!
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
        send_leds(); // set the led's
	}
  }
}




//sensor stuff---------------------------------

// Read all robot sensors into robot.sensor
void read_sensors(void) {

  //dumby sensor values until we get non blocking sensor read going
  static int16_t trig_pin;
 switch (ultraSound.current)  //select pin
 {
   case 1:
     trig_pin = PIN_USOUND1_TRG;
   break;
   case 2:
     trig_pin = PIN_USOUND2_TRG;
   break;
   case 3:
     trig_pin = PIN_USOUND3_TRG;
   break;
   case 4:
     trig_pin = PIN_USOUND4_TRG;
   break;
   case 5:
     trig_pin = PIN_USOUND5_TRG;
   break;
   default:
   ;
   //  digitalWrite(PIN_DEBUG,HIGH); //this should not happen
 }

 switch (ultraSound.state)
 {
   case 0:
   if(!ultraSound.wait)
   {
     digitalWrite(trig_pin,LOW);//start low pulse
     ultraSound.wait=true;
     ultraSound.start=micros();
   }
   else
   {
     if((micros()-ultraSound.start)>2)
     {
       ultraSound.state++;
       ultraSound.wait=false;
     }
   }
   break;

   case 1:
   if(!ultraSound.wait)
   {
     digitalWrite(trig_pin,HIGH);//start pulse
     ultraSound.wait=true;
     ultraSound.start=micros();
   }
   else
   {
     if((micros()-ultraSound.start)>10)
     {
       ultraSound.state++;
       ultraSound.wait=false;
     }
   }
   break;

   case 2:
   if(!ultraSound.wait)
   {
     digitalWrite(trig_pin,LOW);//start low pulse
     ultraSound.wait=true;
     ultraSound.start=micros();
   }
   else
   {
     if((micros()-ultraSound.start)>70820) //time out
     {
       ++ultraSound.current;
       if(ultraSound.current>5)//cycel sensor
         ultraSound.current=1;
       ultraSound.state=0;
       ultraSound.wait=false;
     }
   }
   break;

   default:
   ;     //digitalWrite(PIN_DEBUG,HIGH);
 }
 /*
  robot.f_sensors.uSound1 = millis();
  robot.f_sensors.uSound2 = 100;
  robot.f_sensors.uSound3 = micros();
  robot.f_sensors.uSound4 = 100;
  robot.f_sensors.uSound5 = 400;
  //end of dumby sensor data*/
}

//sends serial sensor packet to PC
void send_sensors(void)
{
  low_latency_ops();
  PC.pkt.write_packet(0x5,sizeof(robot.f_sensors),&robot.f_sensors); //fast sensor
 // robot.f_sensors.latency=0; // reset latency metric
}

//interupt function
void u_received()
{
  int16_t up_down=0;
  static uint32_t start;
  up_down=digitalRead(PIN_USOUND_READ);
  if(up_down){
 // digitalWrite(PIN_DEBUG,HIGH);
    start=micros();
  }
  else
  {
   switch (ultraSound.current) //where we recored data
 {
   case 1:
     robot.f_sensors.uSound1 =(micros()-start)/120;
   break;
   case 2:
     robot.f_sensors.uSound2 =(micros()-start)/120;
   break;
   case 3:
     robot.f_sensors.uSound3 =(micros()-start)/120;
   break;
   case 4:
     robot.f_sensors.uSound4 =(micros()-start)/120;
   break;
   case 5:
     robot.f_sensors.uSound5 =(micros()-start)/120;
   break;
   default:
   ;   //  digitalWrite(PIN_DEBUG,HIGH); //this should not happen
 }
 ultraSound.state=0;
 ultraSound.wait=false;
  ++ultraSound.current;
  if(ultraSound.current>5)//cycel sensor
    ultraSound.current=1;
//  digitalWrite(PIN_DEBUG,LOW);
  }
}








//LED stuff------------------------------------------

//sets LED based on current mode (off on), demo/one color
// command is quick and nonblocking (only "has" to happen on change of robot.led or when demo changes color(5secounds))
void send_leds(void) {
   if(robot.led.ledon)
  {
    if(robot.led.demo)
    {
      LEDdemo();
    }
    else
    {
    analogWrite(ledpins.blue,robot.led.blue);
    analogWrite(ledpins.red,robot.led.red);
    analogWrite(ledpins.green,robot.led.green);
    }
  }
  else
  {
    analogWrite(ledpins.blue,0);
    analogWrite(ledpins.red,0);
    analogWrite(ledpins.green,0);
  }
}


//lights demo code (needs cleaned)
void LEDdemo()
{

static int16_t count=0;
static int16_t countspace=5000;
static uint32_t lastcount=0;
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

//ramps all selected led's (ramp.active) power levels from 0-255
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
