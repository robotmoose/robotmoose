/**
 Set up everything on this machine, according to CSV-style text
 descriptors sent in on the serial port.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-17 (Public Domain)
*/
#include "tabula_config.h"
#include "action.h"
#include "motor_controller.h"
// please add your header here!

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



bool tabula_configure() {
	tabula_configure_source src(Serial);
	String device=src.readString();

	tabula_factory *factory=tabula_factory_find(device);
	
	if (factory) { 	// a normal user device
		factory->create(src);
		if (!src.failure) {
			Serial.print("Added device ");
			Serial.println(device);
		}
	}
	else if (device=="version?") { 
		Serial.println("2015-03-17 Anteater");
	}
	else if (device=="devices?") { 
		tabula_factory_list();
	}
	else if (device=="ram?") { // estimate free RAM remaining
		// See http://playground.arduino.cc/code/AvailableMemory
		extern int __heap_start, *__brkval; 
		int stack_top; 
		int bytes=(int) &stack_top - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
		Serial.print(bytes);
		Serial.println(" bytes free");
	}
	else if (device[0]=='?' || device[0]=='h') { // help/hello/howto
		Serial.println("This interface lets you configure Arduino devices.  Registered devices:");
		tabula_factory_list();
	}
	else if (device=="print!") { // echo one string to screen
		Serial.println(src.readString());
	}
	else if (device=="reset!") { // soft reboot
		void (*reset_vector)() = 0; // fn ptr to address 0
		reset_vector();
	}
	else if (device=="loop!") {
		return false; // we're done!
	}
	else {
		src.failed("no such device",device);
	}

/* Report errors back over serial */
	if (src.failure) {
		Serial.print("ERROR: ");
		Serial.print(src.failure);
		Serial.print(" '");
		Serial.print(src.failedValue);
		Serial.print("' while configuring ");
		Serial.println(device);
	}
	
	return true; // ready for more configurations
}


// Blocking function: configure the device until "loop!" is reached.
void tabula_setup() {
	action_setup();
	
	Serial.begin(57600);
	Serial.println("Please enter configuration strings: device pins...");
	
	while (tabula_configure()) {}
}




