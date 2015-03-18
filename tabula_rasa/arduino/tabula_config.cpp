/**
 Set up everything on this machine, according to CSV-style text
 descriptors sent in on the serial port.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-17 (Public Domain)
*/
#include "tabula_config.h"
#include "action.h"
#include "motor_controller.h"
// please add your header here!


bool tabula_configure() {
	ConfigureSource src(Serial);
	String device=src.readString();

/* Check for and configure every possible device here */

// Motor controllers:
	if (device=="bts_controller_t") {
		 uint16_t left[2], right[2]; 
		 left[0]=src.readPin();	left[1]=src.readPin();
		 right[0]=src.readPin(); right[1]=src.readPin();
		 actions_10ms.add(new bts_controller_t(left,right));
	}
	else if (device=="sabertooth_v1_controller_t") {
		 Stream *saber=src.readSerial(9600);
		 actions_10ms.add(new sabertooth_v1_controller_t(*saber));
	}
	
// Special system objects: these have immediate effects
	else if (device=="version?") { 
		Serial.println("2015-03-17 Anteater");
	}
	else if (device=="ram?") { // estimate free RAM remaining
		// See http://playground.arduino.cc/code/AvailableMemory
		extern int __heap_start, *__brkval; 
		int stack_top; 
		int bytes=(int) &stack_top - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
		Serial.print(bytes);
		Serial.println(" bytes free");
	}
	else if (device=="?" || device=="help") { // howto
		Serial.println("This interface lets you configure Arduino devices.");
	}
	else if (device=="print!") { // echo one string to screen
		Serial.println(src.readString());
	}
	else if (device=="reset!") { // soft reboot
		void (*reset_vector)() = 0; // fn ptr to address 0
		reset_vector();
	}
	else if (device=="run!") {
		return false; // we're done!
	}
	else {
		src.failed("no such device",device);
	}

/* Report status of addition */
	if (!src.failure) {
		Serial.print("Added device ");
		Serial.println(device);
	} else {
		Serial.print("ERROR: ");
		Serial.print(src.failure);
		Serial.print(" '");
		Serial.print(src.failedValue);
		Serial.print("' while configuring ");
		Serial.println(device);
	}
	
	return true; // ready for more configurations
}


// Blocking function: configure the device until "run!" is reached.
void tabula_setup() {
	Serial.begin(57600);
	Serial.println("Please enter configuration strings: device pins...");
	
	while (tabula_configure()) {}
}




