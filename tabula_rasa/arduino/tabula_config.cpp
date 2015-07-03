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

int tabula_factory_count() {
	int count=0;
	for (tabula_factory *cur=tabula_factory::head;cur!=NULL;cur=cur->next) {
		count++;
	}
	return count;
}

// Print all devices
void tabula_factory_list() {
	for (tabula_factory *cur=tabula_factory::head;cur!=NULL;cur=cur->next) {
		Serial.print(cur->device_name);
		Serial.print(" ");
		Serial.println(cur->arg_types);
	}
}

// Find the factory with this name
tabula_factory *tabula_factory_find(const String &device_name) {
	for (tabula_factory *cur=tabula_factory::head;cur!=NULL;cur=cur->next)
		if (device_name == cur->device_name)
			return cur;
	return NULL;
}

// Expect this character
void expect_char(tabula_configure_source &src,char expected) {
	char c=src.read();
	if (c!=expected) { // not the character we wanted
		String err="";
		err+=expected;
		err+=" but got ";
		err+=c;
		src.failed(tabula_flash_string("Expected "),err);
	}
}

// Configure one device
bool tabula_configure() {
	tabula_configure_source src(Serial);
	// Serial.println("reading user string");
	String cmd=src.read_string();

	tabula_factory *factory=tabula_factory_find(cmd);
	
	if (factory) { 	// a normal user device
		expect_char(src,'(');
		if (!src.failure) {
		  factory->create(src);
		  if (!src.failure) {
		    expect_char(src,')');
		    if (!src.failure) {
		      expect_char(src,';');
		      if (!src.failure) {
			Serial.print(F("1 Added device "));
			Serial.println(cmd);
			if (cmd==tabula_flash_string("serial_controller")) 
				return false; // stop configuration and read binary packets from here.
		      }
		    }
		  }
		}
	}
	else if (cmd==tabula_flash_string("version?")) { 
		Serial.println(F("1 2015-07-03 Chugiak"));
	}
	else if (cmd==tabula_flash_string("sensors?")) { 
		Serial.print(F("1  ")); tabula_sensor_storage.print<unsigned char>();
	}
	else if (cmd==tabula_flash_string("commands?")) { 
		Serial.print(F("1  ")); tabula_command_storage.print<unsigned char>();
	}
	else if (cmd==tabula_flash_string("cmd")) { 
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
	else if (cmd==tabula_flash_string("cmd16")) { 
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
	else if (cmd==tabula_flash_string("ram?")) { // estimate free RAM remaining
		// See http://playground.arduino.cc/code/AvailableMemory
		extern int __heap_start, *__brkval; 
		int stack_top; 
		int bytes=(int) &stack_top - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
		Serial.print(bytes);
		Serial.println(F(" bytes free"));
	}
	else if (cmd==tabula_flash_string("list")) { // list devices (clean version for backend)
		Serial.println(tabula_factory_count());
		tabula_factory_list();
	}
	else if (cmd[0]=='?' || cmd[0]=='h') { // help/hello/howto
		Serial.println(F("This is how you configure your Arduino at runtime.  Registered devices:"));
		tabula_factory_list();
		Serial.println(F("Argument format: S is a Serial port, like X3 for TX3/RX3.  P is a Pin, like pin 13, or A3."));
	}
	else if (cmd==tabula_flash_string("reset!")) { // soft reboot
		void (*reset_vector)() = 0; // fn ptr to address 0
		reset_vector();
	}
	else if (cmd==tabula_flash_string("loop!")) {
		Serial.println(tabula_command_storage.count); //size in bytes for backend side to read
		Serial.println(tabula_sensor_storage.count); //size in bytes for backend side to read
		return false; // we're done!
	}
	else {
		src.failed(tabula_flash_string("bad command"),cmd);
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
	Serial.println(F("9 Enter your tabula device names now:"));
	
	while (tabula_configure()) { }
}




