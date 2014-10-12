/**
 * Aurora Robotics "Autonomy" firmware
 * For Arduino Uno, hooked to LAYLA telepresence platform.
 */
#include "robot.h" /* classes shared with PC, and across network */
#include "serial_packet.h" /* CYBER-Alaska packetized serial comms */

#include <SoftwareSerial.h>

SoftwareSerial saberSerial(9,8); // RX (not used), TX


void sendMotor(int motorSide,int power) {
  if (power<1) power=1;
  if (power>126) power=126;
  if (motorSide) power+=128;
  saberSerial.write((unsigned char)power);
}


// All PC commands go via this (onboard USB) port
HardwareSerial &PCport=Serial; // direct PC

// Call this function frequently--it's for minimum-latency operations
void low_latency_ops() {
}

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
CommunicationChannel PC(PCport);

/***************** Robot Control Logic ****************/

void setup()
{
  Serial.begin(9600); // Control connection to PC
  saberSerial.begin(9600); // sabertooth motor controller

  // Our ONE debug LED!
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);

}

// Robot's current state:
robot_current robot;

// Read all robot sensors into robot.sensor
void read_sensors(void) {
  /* no sensors on LAYLA, so far! */
  low_latency_ops();
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
}

unsigned long next_micro_send=0;
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
    next_micro_send=micro+25*1024; // send_motors takes 20ms
  }

  low_latency_ops();
}




