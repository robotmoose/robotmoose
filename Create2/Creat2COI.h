/*C++ library to provide abstraction for the Create Open Interface 2(COI2)
  Arsh Chauhan (achauhan@alaska.edu)
  02/09/2015
  Written for UAF's ITEST Program
*/
/* Code is Released on an as is basis into the Public Domain */


#ifndef CREATE2COI_H_INCLUDED
#define CREATE2COI_H_INCLUDED

#include<arduino.h>
#include<SotwareSerial.h>


//Create startup values
#define DEFAULT_BAUDRATE        57600 // Create needs to talk at this rate first

// Setup Commands
#define CMD_START 		0x80
#define CMD_BAUD		0x81
#define CMD_RESET 		0x7
#define CMD_STOP  		0xAD
#define CMD_MODE_SAFE		0x83
#define CMD_MODE_FULL		0x84

// Set Baud Rate (Needs to be sent after CMD_BAUD
#define BAUD_RATE_9600		0x5

// Drive Commands 
#define DRIVE			0x89
#define DRIVE_DIRECT		0x91
#define DRIVE_PWM		0x92


class COI2
{
	bool uno;
	
	public:
	SoftwareSerial coi2Serial; 
	void serialSetup(int,int);
	void serialWrite(byte);
	void driveMode(int);
	void directDrive(int,int);
	void initialize();
	void stop();
	
};

/* Currently hardcoded to use 9600 Baud
   if using Mega, then use pin 16 (TX) and pin 17 (RX)
*/
void COI2::serialSetup (int rxPin, int txPin)
{
	serialWrite(CMD_START);
	serialWrite	
	if(uno) // Using Arduino Uno
	{
		coi2Serial = SoftwareSerial(rxPin,txPin);
		coi2serial.begin(DEFAULT_BAUDRATE);
	}
	else //Using Mega
	{
		Serial2.begin(DEFAULT_BAUDRATE);
	}
}

void COI2::serialWrite ( byte data)
{
	if(uno)
	{
		coi2Serial.write(byte);
	}
	else
	{
		Serial2.write(byte);
	}
}

void COI2::serialRead()
{
	if(uno)
	{
		coi2Serial.read();
	}
	else
	{
		Serial2.read();
	}
}

/*driveMode:
  1:drive - Both wheels in tandom 
  2:Drive Direct - Control each wheen independently
  3:PWM Drive - PWN drive each wheel independently
*/

void driveMode(int driveMode)
{
	switch(driveMode)
	{
		case 1:serialWrite(DRIVE);
		break;
		case 2:serialWrite(DRIVE_DIRECT);
		break;
		case 3:serialWrite(DRIVE_PWM);
	}
}	

/*
  right: Right wheel velocity in mm/s (-500 - 500)
  left: Left wheel velocity in mm/s (-500 - 500)
*/	

void COI2::directDrive(int rightVelocity, int leftVelocity)
{
	serialWrite(DRIVE_DIRECT);	
	serialWrite(highByte(rightVelocity));
	serialWrite(lowByte(rightVelocity));
	serialWrite(highByte(leftVelocity));
	serialWrite(lowByte(leftVelocity));
}

/* Call this function to open the create interface */
void COI2::initialize(int rxPin, int txPin, int driveMode)
{
	serialSetup(rxPin,txPin);
	serialWrite(CMD_START);
	serialWrite(CMD_BAUD);
	serialWrite(BAUD_9600); // Set baud rate to 9600 baud (eventually this should be configurable)
	serialWrite(CMD_MODE_FULL);
	driveMode(driveMode);
}

/*Call this function to disable the create interface
  Robot will stop responding to all following commands
  Call initialize() again to restart
 */

void COI2::stop()
{
	serialWrite(CMD_STOP);
}

	



	

	


			
		






























#endif
