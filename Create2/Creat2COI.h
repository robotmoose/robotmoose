/*Arduino API for the IRobot Create 2 Create Open Interface (COI2)
  Arsh Chauhan (achauhan@alaska.edu)
  02/09/2015
  Written for UAF's ITEST Program
*/
/* Code is Released on an as is basis into the Public Domain */

/*Development Notice: This API is under heavy development. Not all Roomba features have been implemented */


#ifndef CREATE2COI_H_INCLUDED
#define CREATE2COI_H_INCLUDED

#include<arduino.h>
#include<SotwareSerial.h>


//Create start-up values
#define DEFAULT_BAUDRATE 	57600 // Create needs to talk at this rate first
#define BOOT_TIME			5000

// Setup Commands
#define CMD_START 		0x80
#define CMD_BAUD		0x81
#define CMD_RESET 		0x7
#define CMD_STOP  		0xAD
#define CMD_MODE_SAFE	0x83
#define CMD_MODE_FULL	0x84

// Set Baud Rate (Needs to be sent after CMD_BAUD
#define BAUD_RATE_9600		0x5

// Drive Commands 
#define DRIVE			0x89
#define DRIVE_DIRECT	0x91
#define DRIVE_PWM		0x92

// Cleaning Commands (This is what the COI documentation calls it)
#define POWER_DOWN		0x85 // Power Down the Roomba 

// LED Commands
#define CMD_LED				0x8B
#define HOME_LED			0x04
#define POWER_LED_GREEN		0x00
#define CMD_ASCII			0xA4

//Sensor Commands
#define SENSOR_QUERY_LIST	0x95
#define SENSOR_STREAM		0x94

		


class COI2
{
	static int uno; // 1 for true 
	
	/*LED status variables */
	byte pwrLEDcolor;
	byte pwrLEDintensity;
	
	
	public:
	SoftwareSerial coi2Serial; 
	void serialSetup(int,int);
	void serialWrite(byte);
	int serialRead();
	int serialAvailable();
	void driveMode(int);
	void directDrive(int,int);
	void initialize();
	void stop();
	void powerDown();
	void sendLED();
	void powerLED();
	void sevenSegment(int) // Write ASCII characters to the 7-segment LED display
	int readSensor(byte);
	
};
bool COI2::uno = 1;

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
		coi2Serial.begin(DEFAULT_BAUDRATE);
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

int COI2::serialAvailable()
{
	if(uno) 
		return coi2Serial.available();
	else
		return Serial2.available();
}
		
int COI2::serialRead()
{
	if(uno)
	{
		if(serialAvailable()>0)
		{
			return coi2Serial.read();
		}
	}
	else
	{
		if(serialAvailable()>0)
		{
			return Serial2.read();
		}
	}
}

/*driveMode:
  1:drive - Both wheels in tandem 
  2:Drive Direct - Control each when independently
  3:PWM Drive - PWN drive each wheel independently
*/
 
/* NOTE: Only Drive Direct has been implemented */
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
	{
		// Set Power LED to Green on power ON
		pwrLEDcolor = POWER_LED_GREEN;
		pwrLEDintensity = 0xff;
		powerLED();
	}
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

// Power down the Roomba 
void COI2::powerDown()
{
	serialWrite(POWER_DOWN);
}

/* LED Commands */

/* Command the power LED
   Currently requires pwrLEdcolor and pwrLEDintensity to be set by caller 
*/
void COI2::powerLED()
{
	serialWrite(CMD_LED);
	serialWrite(HOME_LED);
	serialWrite(pwrLEDcolor);
	serialWrite(pwrLEDintesity);
}

/*Write ASCII characters onto the 7-segment LED Display
  Pass it the ASCII value
  Acceptable Values: 32-96, 123-126
*/
void COI2::sevenSegment(int ASCII)
{
	serialWrite(CMD_ASCII);
	serialWrite(ASCII);
]

/* Reading Sensors
   The Roomba updates its sensors every 15ms internally. This is a design decision made by IRobot
*/	
/*Read a single sensor */
int COI2::readSensor(byte sensor)
{
	int data;
	serialWrite(SENSOR_QUERY_LIST);
	serialWrite(0x01); // read 1 packet
	serialWrite(sensor);
	data = serialRead();
	return data;
}

/*Read Specified sensors every 15 ms
  Suggested method for reading over network
*/

/* 
  Development Note: Read yet to be implemented 
*/
int COI2::sensorStream(byte sensors[])
{
	int packets = sizeof(sensors);
	serialWrite(SENSOR_STREAM);
	byte numBytes = byte(packets);
	serialWrite(numBytes);
	for(int i=0;i<packets;i++)
	{
		byte byteCode = byte(sensors[i]);
		serialWrite(byteCode);
	}
	
	// Implement Read stuff here
}
	
		

			
		






























#endif
