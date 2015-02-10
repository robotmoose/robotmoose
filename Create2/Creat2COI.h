#ifndef CREATE2COI_H_INCLUDED
#define CREATE2COI_H_INCLUDED

#include<arduino.h>
#include<SotwareSerial.h>

// Setup Commands
#define CMD_START 				0x80;
#define CMD_BAUD				0x81;
#define CMD_RESET 				0x7;
#define CMD_STOP  				0xAD;
#define CMD_MODE_SAFE			0x83;
#define CMD_MODE_FULL			0x84;

// Set Baud Rate (Needs to be sent after CMD_BAUD
#define BAUD_RATE_9600				0x5;

// Drive Commands 
#define DRIVE					0x89;
#define DRIVE_DIRECT			0x91;
#define DRIVE_PWM				0x92;


class COI2serial
{
	bool uno;
	
	public:
	void serialSetup(int,int);
	void serialWrite(byte);

/* Currently hardcoded to use 9600 Baud
   if using Mega, then use pin 16 (TX) and pin 17 (RX)
*/
void COI2SerialSetup (bool uno, int rxPin, int txPin)
{
		if(uno) // Using Arduino Uno
		{
			SoftwareSerial coi2Serial = SoftwareSerial(rxPin,txPin);
			coi2serial.begin = (BAUD_RATE_9600);
		}
		else //Using Mega
		{
			Serial2.begin(BAUD_RATE_9600);
		}
}

void COISerialwrite ( byte data)
{
	
		






























#endif
