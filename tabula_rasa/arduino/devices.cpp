/**
 Example devices using the tabula_command, tabula_sensor,
 REGISTER_TABULA_DEVICE, and action interfaces.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-18 (Public Domain)
*/
#include <Servo.h>
#include <Wire.h>
#include "tabula_control.h"
#include "tabula_config.h"

// Servo output example:
class servo_device : public action {
public:
	Servo s; // the Servo
	tabula_command<unsigned char> deg; // commanded degree orientation
	
	virtual void loop() {
		s.write(deg);
	}
};

REGISTER_TABULA_DEVICE(servo,
	int pin=src.read_pin();
	servo_device *device=new servo_device();
	device->s.attach(pin);
	src.command_index("deg",F("commanded orientation in degrees"),device->deg.get_index());
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

// Battery Management System
class BMS : public action {
public:
        enum { bms_addr = 2 }; // Address of BMS on I2C
        enum { numbytes = 2 }; // # of bytes to request from BMS
	virtual void loop() {
		Wire.requestFrom(bms_addr, numbytes);
                while(Wire.available() <= 2 && Wire.available() >= 1)    // slave may send less than requested
                {
                           byte x = Wire.read(); // receive a byte as character
                           Serial.print(x);   // print the character
                           Serial.println("");
                }

	}	
};

REGISTER_TABULA_DEVICE(BMS,
        Wire.begin();
	BMS *device=new BMS();
	actions_10s.add(device);
)
    

// Watch sensor or command values on serial port (in ASCII)
template <class T>
class sensor_watcher : public action { public:
	virtual void loop() {
		Serial.print(F("Sensors: "));
		tabula_sensor_storage.print<T>();
	}	
};
REGISTER_TABULA_DEVICE(sensors8, actions_1s.add(new sensor_watcher<unsigned char>());  )
REGISTER_TABULA_DEVICE(sensors16, actions_1s.add(new sensor_watcher<int16_t>());  )

template <class T>
class command_watcher : public action { public:
	virtual void loop() {
		Serial.print(F("Commands: "));
		tabula_command_storage.print<T>();
	}	
};
REGISTER_TABULA_DEVICE(commands8, actions_1s.add(new command_watcher<unsigned char>());  )
REGISTER_TABULA_DEVICE(commands16, actions_1s.add(new command_watcher<int16_t>());  )


// Simple ASCII serial heartbeat
class heartbeat : public action {
public:
   void loop() { Serial.println("Heartbeat!"); }
};

REGISTER_TABULA_DEVICE(heartbeat, 
  actions_1s.add(new heartbeat());
)


// Neato XV-11 laser distance sensor
#include "neato_serial.h"

class neato : public action {
public:
	NeatoLDS<Stream> n;
	int neatoMotorPin;
	neato(Stream &s,int motorPin_) 
		:n(s), neatoMotorPin(motorPin_)
	{
		
	}
	
	void loop() {
		// Incoming comms
		int leash=100; // bound maximum latency
		while (n.read()) { if (--leash<0) break; }
		
		// Motor control
		pinMode(neatoMotorPin,OUTPUT);
		//if (PC_connected) {
			if (n.lastBatch.speed64>200.0) { // PWM
				int target=280; // target RPM
				int err=n.lastBatch.speed64/64-target;
				analogWrite(neatoMotorPin,255-constrain(err*10 + 30, 0, 255));
			} else { // simple hardcoded startup value
				analogWrite(neatoMotorPin,255);
			}

		//} else { // PC not connected
		//	analogWrite(neatoMotorPin,0); // turn motor off
		//}
		
		// Outgoing comms?
		//  FIXME!
	}
};

REGISTER_TABULA_DEVICE(neato, 
	Stream *s=src.read_serial(115200);
	int motorPin=src.read_pin();
	neato *n=new neato(*s,motorPin);
	actions_10ms.add(n);
)


