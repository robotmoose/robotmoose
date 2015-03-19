/**
 Set up everything on this machine, according to CSV-style text
 descriptors sent in on the serial port.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-17 (Public Domain)
*/
#include "tabula_config.h"
#include "tabula_control.h"
#include "action.h"

// First element of list:
tabula_factory *tabula_factory::head=0;


// Print all devices
void tabula_factory_list() {
	for (tabula_factory *cur=tabula_factory::head;cur!=NULL;cur=cur->next)
		Serial.println(cur->device_name);
}

// Find the factory with this name
tabula_factory *tabula_factory_find(const String &device_name) {
	for (tabula_factory *cur=tabula_factory::head;cur!=NULL;cur=cur->next)
		if (device_name == cur->device_name)
			return cur;
	return NULL;
}


// Configure one device
bool tabula_configure() {
	tabula_configure_source src(Serial);
	String cmd=src.read_string();

	tabula_factory *factory=tabula_factory_find(cmd);
	
	if (factory) { 	// a normal user device
		factory->create(src);
		if (!src.failure) {
			Serial.print(F("1 Added device "));
			Serial.println(cmd);
		}
	}
	else if (cmd=="devices?") { 
		tabula_factory_list();
	}
	else if (cmd=="version?") { 
		Serial.println(F("1 2015-03-18 Anchorage"));
	}
	else if (cmd=="sensors?") { 
		Serial.print(F("1  ")); tabula_sensor_storage.print<unsigned char>();
	}
	else if (cmd=="commands?") { 
		Serial.print(F("1  ")); tabula_command_storage.print<unsigned char>();
	}
	else if (cmd=="cmd") { 
		int index=src.read_int();
		int value=src.read_int();
		if (!src.failure) {
			tabula_command_storage.array[index]=value;
			Serial.print(F("1  set command index "));
			Serial.print(index);
			Serial.print(F(" to value "));
			Serial.println(value);
		}
	}
	else if (cmd=="cmd16") { 
		int index=src.read_int();
		long value=src.read_int();
		if (!src.failure) {
			tabula_command_storage.array[index]=value;
			tabula_command_storage.array[index+1]=value>>8;
			Serial.print(F("1  set 16 bit command index "));
			Serial.print(index);
			Serial.print(F(" to value "));
			Serial.println(value);
		}
	}
	else if (cmd=="ram?") { // estimate free RAM remaining
		// See http://playground.arduino.cc/code/AvailableMemory
		extern int __heap_start, *__brkval; 
		int stack_top; 
		int bytes=(int) &stack_top - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
		Serial.print(bytes);
		Serial.println(F(" bytes free"));
	}
	else if (cmd[0]=='?' || cmd[0]=='h') { // help/hello/howto
		Serial.println(F("This interface lets you configure Arduino devices.  Registered devices:"));
		tabula_factory_list();
	}
	else if (cmd=="print!") { // echo one string to screen
		Serial.println(src.read_string());
	}
	else if (cmd=="reset!") { // soft reboot
		void (*reset_vector)() = 0; // fn ptr to address 0
		reset_vector();
	}
	else if (cmd=="loop!") {
		return false; // we're done!
	}
	else {
		src.failed("no such device or command",cmd);
	}

/* Report errors back over serial */
	if (src.failure) {
		Serial.print(F("0 ERROR: "));
		Serial.print(src.failure);
		Serial.print(F(" '"));
		Serial.print(src.failed_value);
		Serial.print(F("' while configuring "));
		Serial.println(cmd);
	}
	
	return true; // ready for more configurations
}


// Blocking function: configure the device until "loop!" is reached.
void tabula_setup() {
	action_setup();
	
	Serial.begin(57600);
	Serial.println(F("9 Please enter configuration strings: device pins..."));
	
	while (tabula_configure()) {}
}




