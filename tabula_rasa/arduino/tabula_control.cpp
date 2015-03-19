/**
 Scalable command and sense system for robot control:

command: PC sending control commands for motor controllers, 
  enable bits, etc.

sensor: Arduino sending temperature, ultrasonic, bumpers, etc.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-18 (Public Domain)
*/
#include "tabula_control.h"
#include "tabula_config.h"

// Static value storage:
tabula_control_storage tabula_sensor_storage;
tabula_control_storage tabula_command_storage;


// Simple analog input pin example:
class analog_sensor : public action {
public:
	int pin; // I/O pin used
	tabula_sensor<uint16_t> value; // read-in value
	
	virtual void loop() {
		value=analogRead(pin);
	}	
};

REGISTER_TABULA_DEVICE(analog_sensor,
	int pin=src.read_pin();
	analog_sensor *device=new analog_sensor();
	device->pin=pin;
	src.sensor_index("value",F("analog input value from 0 to 1023"),device->value.get_index());
	actions_10ms.add(device);
)

// Simple "pwm_pin" PWM output pin example:
class pwm_pin : public action {
public:
	int pin; // I/O pin used
	tabula_command<unsigned char> pwm; // commanded PWM strength
	
	virtual void loop() {
		analogWrite(pin,pwm.get());
	}	
};

REGISTER_TABULA_DEVICE(pwm_pin,
	int pin=src.read_pin();
	pwm_pin *device=new pwm_pin();
	device->pin=pin;
	src.command_index("pwm",F("strength from 0 (off) to 255 (on)"),device->pwm.get_index());
	actions_10ms.add(device);
)

// Simple "ramp" meta-command example:
//   Updates current command array at specified index
class ramp : public action {
public:
	int index; // command index to ramp
	tabula_command<signed char> delta; // change per 100 milliseconds
	
	virtual void loop() {
		tabula_command_storage.array[index]+=delta.get();
	}
};

REGISTER_TABULA_DEVICE(ramp,
	int index=src.read_int();
	ramp *device=new ramp;
	device->index=index;
	src.command_index("delta",F("change to apply per 100 milliseconds"),device->delta.get_index());
	actions_100ms.add(device);
)

// Watch sensor or command values on serial port
template <class T>
class sensor_watcher : public action { public:
	virtual void loop() {
		Serial.print(F("Sensors: "));
		tabula_sensor_storage.print<T>();
	}	
};
REGISTER_TABULA_DEVICE(sensors8, actions_1s.add(new sensor_watcher<unsigned char>());  )
REGISTER_TABULA_DEVICE(sensors16, actions_1s.add(new sensor_watcher<uint16_t>());  )

template <class T>
class command_watcher : public action { public:
	virtual void loop() {
		Serial.print(F("Commands: "));
		tabula_command_storage.print<T>();
	}	
};
REGISTER_TABULA_DEVICE(commands8, actions_1s.add(new command_watcher<unsigned char>());  )

