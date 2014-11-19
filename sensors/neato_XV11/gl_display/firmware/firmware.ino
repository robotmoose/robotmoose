/**
  Neato XV-11 Laser Distance Sensor (LDS) display and interface code.
  
  This version accumulates batched reports at the Arduino side, and
  sends completed reports to the PC.
  
  Hardware: Arduino Mega and NEATO XV-11 sensor
  Wiring:
  	Green motor control wire: pin 4 (neatoMotorPin)
  	Orange serial input wire: RX3 (Serial3)
  	Power & ground to Vin
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-02-19 (Public Domain)
*/
#include "serial_packet.h"
#include "neato_serial.h"


/**************** Interface Side ***************/
NeatoLDS<HardwareSerial> neato(Serial3);
int neatoMotorPin=4; // PWM control for scanner motor

HardwareSerial &PCserial=Serial;

A_packet_formatter<HardwareSerial> PCpacket(PCserial);

unsigned long last_command=0;
unsigned long last_send=0;
unsigned char PC_connected=0;

void setup(void) {
  neato.begin();
  PCserial.begin(57600); // for XBee compatibility
  
  
  for (int pin=2;pin<=7;pin++)
  {
     pinMode(pin,OUTPUT);
     digitalWrite(pin,LOW);
  }
}

void loop(void) {
  #if 0 /* Sweep motor power levels: */
  for (int power=0;power<255;power+=10) {
      analogWrite(2,power);
      
      for (int d=0;d<2000;d++) {
        while (neato.read()) {  }
        delay(1);
      }
      
      DebugSerial_write("Experiment power: ");
      DebugSerial_print(power);
    DebugSerial_write(" rpm ");
    DebugSerial_print(neato.speed/64);
    DebugSerial_write("\n");
  }
  #endif
  
  #if 0 /* PID tuning experiment */
  for (int prop=0;prop<255;prop++) {
      
      int power;
      int pmax=0,pmin=255;
      int rmax=0,rmin=1024;
      for (int d=0;d<2000;d++) {
        while (neato.read()) {  }
        delay(1);
        int speed=neato.speed/64;
        int target_speed=300; // rpm
        int err=speed-target_speed;
        power=100+(err*prop);
        if (power<0) power=0;
        if (power>255) power=255;
        
        if (pmax<power) pmax=power;
        if (pmin>power) pmin=power;
        
        if (rmax<speed) rmax=speed;
        if (rmin>speed) rmin=speed;
        
        analogWrite(2,power);
      }
      
      DebugSerial_write("Experiment prop: ");
      DebugSerial_print(prop);
      
    DebugSerial_write(" power ");
    DebugSerial_print(power);
    DebugSerial_write(" range ");
    DebugSerial_print(pmin);
    DebugSerial_write("-");
    DebugSerial_print(pmax);
      
    DebugSerial_write(" rpm range ");
    DebugSerial_print(rmin);
    DebugSerial_write("-");
    DebugSerial_print(rmax);
    
    DebugSerial_write(" rpm ");
    DebugSerial_print(neato.speed/64);
    DebugSerial_write("\n");
  }
  #endif
  
  


   int leash=100;

/* Neato sensor interfacing */
   digitalWrite(2,HIGH);
   while (neato.read()) { if (--leash<0) break; }
   digitalWrite(2,LOW);

/* PC requests */
   A_packet p;
   leash=100;
   while (PCpacket.read_packet(p)==-1) { PC_connected=1; if (--leash<0) break; }
   
   
   if (p.valid) { // handle PC request
   	last_command=last_send=millis();
   	if (p.command==0xD) // request distance report
   		PCpacket.write_packet(0xD,sizeof(neato.lastBatch),&neato.lastBatch);
        else
                PCpacket.write_packet(0xE,1,&p.command); // echo bad commands with 0xE error code
   }

/* Timeout heartbeat */
   digitalWrite(6,HIGH);
   if (last_send+500<millis()) 
   { // no request in a while--send heartbeat
     PC_connected=0;
   //  static int heart_state=1;
   //digitalWrite(13,heart_state); heart_state=!heart_state; // blink LED 13
  
  // PCserial.begin(57600); // reset serial port?
  
      last_send=millis();
      PCpacket.reset(); // if we timed out, there's no packet coming
   	PCpacket.write_packet(0,0,0); // basic heartbeat (OK)
   }
   digitalWrite(6,LOW);


/* Neato motor speed control */
   pinMode(neatoMotorPin,OUTPUT);
   if (PC_connected) {
     if (neato.lastBatch.speed64>200.0) { // PWM
       int target=280; // target RPM
       int err=neato.lastBatch.speed64/64-target;
       analogWrite(neatoMotorPin,constrain(err*10 + 30, 0, 255));
     } else { // simple hardcoded value
       analogWrite(neatoMotorPin,0);
     }
     
   } else { // PC not connected
      analogWrite(neatoMotorPin,255); // turn motor off
   }
}

