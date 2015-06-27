/**
 * Aurora Robotics "Autonomy" firmware
 * For Arduino Uno, hooked to LAYLA telepresence platform.
 */
#include "robot.h" /* classes shared with PC, and across network */
#include "serial_packet.h" /* CYBER-Alaska packetized serial comms */
void handle_packet(A_packet_formatter<HardwareSerial> &pkt,const A_packet &p);  //reads serial packets, and sometimes read/sends sensors

#define PIN_DEBUG 13
HardwareSerial &robotserial=Serial3; // RX (not used), TX
HardwareSerial &PCport=Serial; // direct PC

//robot constants
#define BOOT_TIME 5000
#define CMD_START               0x80
#define CMD_MODE_SAFE           0x83
#define CMD_MODE_FULL           0x84
#define CMD_MODE_PASSIVE        0x80
#define CMD_DRIVE_DIRECT        0x91


class irobot
{
  public:
   void start(void);
   void motors(int right,int left);
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

// All PC commands go via this (onboard USB) port
CommunicationChannel PC(PCport);
 
// Robot's current state:
robot_current robot;
irobot create2;




void setup()
{
  PCport.begin(9600); // Control connection to PC
    // Our ONE debug LED!
  pinMode(PIN_DEBUG,OUTPUT);
  digitalWrite(PIN_DEBUG,LOW);
  create2.start();
  delay (1000);
}

void loop()
{
 A_packet p;
  if (PC.read_packet(millis(),p)) handle_packet(PC.pkt,p);
 if (!(PC.is_connected)) robot.power.stop(); // disconnected?
}







 void irobot::start()
 {
   //delay(BOOT_TIME);
   robotserial.begin(115200); //defualt
   robotserial.write(0x7);
   delay(BOOT_TIME);
   robotserial.write(CMD_START);
   robotserial.write(CMD_MODE_FULL);
   //set other start stuff ie led what not
 }
 
 void irobot::motors(int right,int left)
 {
    robotserial.write(CMD_DRIVE_DIRECT);
    robotserial.write(highByte (right));
    robotserial.write(lowByte (right));
    robotserial.write(highByte (left));
    robotserial.write(lowByte (left));
 }

void handle_packet(A_packet_formatter<HardwareSerial> &pkt,const A_packet &p)
{
  if (p.command==0x7) { // motor power commands
    if (!p.get(robot.power)) { // error
      pkt.write_packet(0xE,0,0);
    }
    else //packet is good
    { 
        int left = (robot.power.left-64)*500./63.;

  int right = (robot.power.right-64)*500./63.;
  create2.motors(right,left); 

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
      digitalWrite(PIN_DEBUG,robot.led.ledon); // DEBUG

	}
  }
}

