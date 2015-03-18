/**
 Convert string object descriptors into actual runnable objects.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-17 (Public Domain)
*/
#ifndef __TABULA_CONFIG_H
#define __TABULA_CONFIG_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "action.h"

/**
 This class supplies configuration data such as strings,
 ints, etc.  Currently it reads these from a serial port in ASCII.
*/
class ConfigureSource {
	Stream &s; // serial input stream
public:
	ConfigureSource(Stream &s_) :s(s_), failure(NULL)
	{}

	/**
	 This flag is used for error reporting.
	 If it's not NULL, it's a human-readable string
	 describing what went wrong.
	*/
	const char *failure;
	
	// This is the string we were parsing before the error.
	String failedValue;
	
	/* A parse error happened--store the error internally */
	void failed(const char *failure_,const String &failedValue_) {
		failure=failure_; failedValue=failedValue_;
	}

	/** 
	Read an Arduino String object from this stream,
	typically a serial port connected to the PC.
	Blocks until a delimiter is reached.
	Stops at spaces or commas, and ignores quotes, like csv.
	*/
	String readString() {
		String ret="";
		while (true) {
			int c=s.read(); // read one char
			if (c==-1) 
			{ // no char to read right now--run actions in the meantime.
				action_loop();
			}
			else if (c=='"' || c=='\'') 
			{ /* ignore quotes (optional in CSV) */ }
			else if (c==',' || c==' ' || c=='\n' || c=='\r') 
			{ /* delimiter */
				if (ret!="")
					return ret; // done with string
				/* else ignore spare delimiters */
			}
			else ret+=(char)c; // add to string
		}
	}

	/**
	Read an integer from this stream object.
	Returns zero if the integer could not be read.
	*/
	long readInt() {
		String str=readString();
		long ret=str.toInt();
		if (ret==0 && str[0]!='0') failed("Error reading integer",str);
		return ret;
	}

	/**
	Read an integer pin number, for an I/O pin.
	Returns zero if the pin could not be read.
	*/
	long readPin() {
		String str=readString(); str.toUpperCase();
		bool is_analog=false;
		if (str[0]=='A') {
			is_analog=true;
			str=str.substring(1); // trim off "A" part
		}
		long pin=str.toInt();
		
		long maxPin=1000, maxAnalog=100;
	#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		maxPin=53; maxAnalog=16;
	#elif	defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
		maxPin=13; maxAnalog=6;
	#endif
		if (is_analog) {
			if (pin<=maxAnalog) pin+=A0;
			else {failed("Invalid analog pin number",str); return 0;}
		}
		else if (pin<=0 || pin>maxPin) {failed("Invalid pin number",str); return 0;}

		return pin;
	}

	/**
	Read a description of a serial port, 
		and initialize it to this baud rate.
	*/
	Stream *readSerial(long baud=0) {
		String str=readString(); str.toUpperCase();

	#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	/* Mega has 4 hardware serial ports, like "X1" for Serial1 (RX1/TX1) */
		HardwareSerial *ret=NULL;
		if (str[0]=='X') {
			switch(str[1]) {
			case '1': ret=&Serial1; break;
			case '2': ret=&Serial2; break;
			case '3': ret=&Serial3; break;
			default: break;
			}
		}
		if (ret==NULL) failed("Error reading Arduino Mega serial port (like X1)",str);
	#else 
	/* Arduino UNO?	Use software serial, like "pins 8 9" (pin 8 RX, pin 9 TX) */
		SoftwareSerial *ret=NULL;
		if (str=="PINS") {
			long rx=readPin();
			long tx=readPin();
			if (rx!=0 && tx!=0)
				 ret=new SoftwareSerial(rx,tx); // FIXME: how can we ever delete this object?
		}
		else failed("Error reading Arduino Uno software serial port (like 'pins 8 9')",str);
	#endif

		if (ret!=NULL && baud>0)
		{
			ret->begin(baud);
		}
		return ret;
	}

};

extern void tabula_setup();

#endif

