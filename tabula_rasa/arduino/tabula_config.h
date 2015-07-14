/**
 Convert string object descriptors into actual runnable objects.
This involves registering object "factories", and parsing serial
commands to find the right factory and call it.

The main things most people need to call are the 
REGISTER_TABULA_DEVICE macro, and the tabula_configure_source object.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-17 (Public Domain)
*/
#ifndef __TABULA_CONFIG_H
#define __TABULA_CONFIG_H

#include <SoftwareSerial.h>
#include <Arduino.h>
#include "action.h"


/**
 On low-RAM microcontrollers like the Arduino Uno, we store strings
 using a special __FlashStringHelper* 
*/
#ifdef __AVR__
#  define tabula_flash_string_ptr const __FlashStringHelper *
#  define tabula_flash_string(str) F(str)

// Compare strings with operator==, by reading each byte from program RAM
inline bool operator==(const String &a,const __FlashStringHelper *b) {
  const char *pa=a.c_str();
  const char /*PROGMEM*/ *pb = (const char /*PROGMEM*/ *)b;
  while (1) {
    unsigned char ac=*pa++;
    unsigned char bc=pgm_read_byte(pb++);
    if (ac!=bc) return false;
    if (ac==0) return true; 
  }
}

#else
#  define tabula_flash_string_ptr const char *
#  define tabula_flash_string(str) str
#endif



/**
 This class supplies configuration data such as strings, ints, etc
 to create devices.
 
 Currently it reads these from a serial port in ASCII.
*/
class tabula_configure_source {
	Stream &s; // serial input stream
public:
	char ungot; // the "ungotten" character, put back in the stream (or 0)
	tabula_configure_source(Stream &s_) :s(s_), ungot(0), failure(NULL)
	{ }

	/**
	  Read the next real char from this device.
	*/
	char read(void) {
		// Try unget buffer
		if (ungot!=0) {
			char c=ungot;
			ungot=0;
			return c;
		}
		// Try serial port
		while (true) {
			int c=s.read(); // read one char
			if (c<=0) 
			{ // no char to read right now--run actions in the meantime.
//Serial.println(F("wait"));
				action_loop();
			}
			else {
// Serial.print(F("Returning char "));
// Serial.println((char)c);
				return c;
			}
		}
	}

	/**
	 This flag is used for error reporting.
	 If it's not NULL, it's a human-readable string
	 describing what went wrong.
	*/
	tabula_flash_string_ptr failure;
	
	// This is the string we were parsing before the error.
	String failed_value;
	
	/* A parse error happened--store the error internally */
	void failed(tabula_flash_string_ptr failure_,const String &failed_value_) {
		failure=failure_; failed_value=failed_value_;
	}

	/** 
	Read an Arduino String object from this stream,
	typically a serial port connected to the PC.
	Blocks until a delimiter is reached.
	Stops at spaces or commas, and ignores quotes, like csv.
	*/
	String read_string() {
		String ret="";
		while (true) {
			char c=read(); // read one char
			if (c=='"' || c=='\'') 
			{ /* ignore quotes (optional in CSV) */ }
			else if (c==' ' || c=='\n' || c=='\r' || c=='(' || c==',' || c==')' || c==';') 
			{ /* delimiter */
				if (c=='(' || c==')' || c==';') ungot=c; // put back delimeters
				if (ret!="") {
// Serial.print(F("Returning string "));
// Serial.println(ret);
					return ret; // done with string
				}
				/* else ignore spare delimiters */
			}
			else ret+=(char)c; // add to string
		}
	}

	/**
	Read an integer from this stream object.
	Returns zero if the integer could not be read.
	*/
	long read_int() {
		String str=read_string();
		long ret=str.toInt();
		if (ret==0 && str[0]!='0') failed(tabula_flash_string("bad int"),str);
		return ret;
	}

	/**
	Read an integer pin number, for an I/O pin.
	Returns zero if the pin could not be read.
	*/
	long read_pin() {
		String str=read_string(); str.toUpperCase();
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
			else {failed(tabula_flash_string("bad analog pin "),str); return 0;}
		}
		else if (pin<=0 || pin>maxPin) {failed(tabula_flash_string("bad pin "),str); return 0;}

		return pin;
	}

	/**
	Read a description of a serial port, 
		and initialize it to this baud rate.
	*/
	Stream *read_serial(long baud=0) {
		String str=read_string(); str.toUpperCase();

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
		if (ret==NULL) failed(tabula_flash_string("bad Mega serial "),str);
	#else 
	/* Arduino UNO?	Use software serial, like "pins 8 9" (pin 8 RX, pin 9 TX) */
		SoftwareSerial *ret=NULL;
		if (str=="PINS") {
			long rx=read_pin();
			long tx=read_pin();
			if (rx!=0 && tx!=0)
				 ret=new SoftwareSerial(rx,tx); // FIXME: how can we ever delete this object?
		}
		else failed(tabula_flash_string("bad sw serial "),str);
	#endif

		if (ret!=NULL && baud>0)
		{
			ret->begin(baud);
		}
		return ret;
	}

	// Report this sensor value index back up to the control PC.  
	// Name is a short one-word value, like "range".
	// Description is human readable, like "distance in mm from ultrasonic sensor"
	void sensor_index(const char *name,const __FlashStringHelper *description,int index) {
		s.print(F("2 index "));
		s.print(index);
		s.print(F(" sensor "));
		s.print(name);
		s.print(F(": "));
		s.println(description);
	}

	// Report this command value index back up to the control PC.  
	// Name is a short one-word value, like "power".
	// Description is human readable, like "motor power, from -255 to +255"
	void command_index(const char *name,const __FlashStringHelper *description,int index) {
		s.print(F("3 index "));
		s.print(index);
		s.print(F(" command "));
		s.print(name);
		s.print(F(": "));
		s.println(description);
	}
};

/** This factory is used by tabula_setup to create
 instances of your device when requested.  
 These classes are typically singletons,
 and are automatically registered when created.

Use REGISTER_TABULA_DEVICE to create your factory.
*/
class tabula_factory {
public:
	// The machine-readable string config name of this device, like "neato".
	//   By convention, it's a shortened version of the name of the C++ class.
	tabula_flash_string_ptr device_name;
	
	// This string describes our configuration arguments, like:
	//   P for a pin
	//   S for a serial port
	// For example, "SP" for one serial port, then one pin;
	//  "PPPP" for four pins.
	tabula_flash_string_ptr arg_types;
	
	tabula_factory(tabula_flash_string_ptr device_name_,tabula_flash_string_ptr arg_types_)
		:device_name(device_name_), arg_types(arg_types_) 
	{
		register_factory(this);
	}
	virtual ~tabula_factory() {}
	
	/// Create your device based on these configuration parameters.
	virtual void create(tabula_configure_source &src) =0;

	/// Link this factory into our list
	static void register_factory(tabula_factory *f) {
		f->next=head;
		head=f;
	}

	static tabula_factory *head;
	tabula_factory *next;
};

/**
  This macro is used to register your device with tabula_setup.
  Use this pattern at file scope:

REGISTER_TABULA_DEVICE(my_motor_controller,"P",
	// This code can use src, a tabula_configure_source
	int outputPin=src.read_pin();
	actions_10ms.add(new my_motor_controller(outputPin));
)
*/
#define REGISTER_TABULA_DEVICE(name, arg_types, create_code) \
	class name##_factory : public tabula_factory { \
	public: \
		name##_factory() :tabula_factory(tabula_flash_string(#name),tabula_flash_string(arg_types)) {} \
		virtual void create(tabula_configure_source &src) { create_code ; } \
	}; \
	const static name##_factory name##_factory_singleton;


extern void tabula_setup();

#endif

