/**
 Example devices using the tabula_command, tabula_sensor,
 REGISTER_TABULA_DEVICE, and action interfaces.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-18 (Public Domain)
*/
#include <Servo.h>
#include <Wire.h>
#include "tabula_control.h"
#include "tabula_config.h"
#include <stdint.h>
#include <Adafruit_NeoPixel.h>


// Servo output example:
class servo_device : public action {
public:
	Servo s; // the Servo
	tabula_command<unsigned char> deg; // commanded degree orientation

	virtual void loop() {
		s.write(deg);
	}
};

REGISTER_TABULA_DEVICE(servo,"P",
	int pin=src.read_pin();
	servo_device *device=new servo_device();
	device->s.attach(pin);
	src.command_index("deg",F("commanded orientation in degrees"),device->deg.get_index());
	actions_10ms.add(device);
)

//NeoPixel
class neo_pixel: public action {
public:
	Adafruit_NeoPixel npix;
	neo_pixel(const int pin): npix(10, pin, NEO_GRB + NEO_KHZ800){}//hardcoded for testing


	virtual void loop(){
		for(int i=0; i<10; i++)
		{
			npix.setPixelColor(i, npix.Color(0, 0, 255));
		}
		npix.show();
	}
};

REGISTER_TABULA_DEVICE(neo_pixel,"P",
	int pin=src.read_pin();
	neo_pixel *device=new neo_pixel(pin);
	device->npix.begin();
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

REGISTER_TABULA_DEVICE(pwm,"P",
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

REGISTER_TABULA_DEVICE(analog,"P",
	int pin=src.read_pin();
	analog_sensor *device=new analog_sensor();
	device->pin=pin;
	src.sensor_index("value",F("analog input value from 0 to 1023"),device->value.get_index());
	actions_10ms.add(device);
)

// Battery Management System
class BMS : public action {
public:
	tabula_sensor<unsigned char> charge; // percent charge, 0-100 percent
	tabula_sensor<unsigned char> state; // current charge state
	enum { bms_addr = 2 }; // Address of BMS on I2C

	// Packet is 1 byte header, 4 byte float for battery percent, 1 byte for charge/discharge info  
	enum { numbytes = (3*sizeof(byte)) }; // # of bytes to request from BMS

	BMS() {
		charge=0;
		state=0; // 0: BMS not connected
	}

	virtual void loop() {
		Wire.requestFrom(bms_addr, numbytes);

		uint32_t battery_percent=0;
		if( Wire.available() ){
		if( Wire.read() == 0xCC ) // If the I2C packet header is 0xCC
		{   
			// Read in float for battery percent from I2C
			/*for(int i=0; i<sizeof(float); ++i)
			{
				battery_percent = battery_percent | (Wire.read() << sizeof(char)*i);
			}
			charge = (float)battery_percent; // Convert float for battery percent from I2C*/
			charge = Wire.read();

			// Bit 0 is 1 if cell #1 is discharging, 0 otherwise
			// Bit 1 is 1 if cell #2 is discharging, 0 otherwise
			// Bit 2 is 1 if cell #3 is discharging, 0 otherwise
			// Bit 3 is 1 if the battery is charging, 0 otherwise
			// Bit 4 is 1 if BMS is OK, 0 otherwise.
			// Bits 5-7 are unused	
			state = Wire.read();
		}
		else
		{
			charge = 54;
			state = 107;
		}
	}
	}
};

REGISTER_TABULA_DEVICE(bms,"",
        Wire.begin();
	BMS *device=new BMS();
	actions_1s.add(device);
)

/*
// Watch sensor or command values on serial port (in ASCII)
template <class T>
class sensor_watcher : public action { public:
	virtual void loop() {
		Serial.print(F("Sensors: "));
		tabula_sensor_storage.print<T>();
	}
};
REGISTER_TABULA_DEVICE(sensors8,"", actions_1s.add(new sensor_watcher<unsigned char>());  )
REGISTER_TABULA_DEVICE(sensors16,"", actions_1s.add(new sensor_watcher<int16_t>());  )

template <class T>
class command_watcher : public action { public:
	virtual void loop() {
		Serial.print(F("Commands: "));
		tabula_command_storage.print<T>();
	}
};
REGISTER_TABULA_DEVICE(commands8,"", actions_1s.add(new command_watcher<unsigned char>());  )
REGISTER_TABULA_DEVICE(commands16,"", actions_1s.add(new command_watcher<int16_t>());  )
*/

// Sensor heartbeat: simple increasing value
class heartbeat : public action {
public:
   unsigned char count;
   tabula_sensor<unsigned char> heartbeats;
   void loop() { heartbeats=count++; }
};

REGISTER_TABULA_DEVICE(heartbeat,"",
  actions_100ms.add(new heartbeat());
)

// Latency monitor
class latency_monitor : public action {
public:
  typedef unsigned short milli_t;
  milli_t the_time() { return micros()>>10; }

  milli_t worst; // worst latency measured so far
  milli_t last; // time at last loop call
  tabula_sensor<unsigned char> latency;
  void loop() {
    milli_t cur=the_time();
    milli_t d=cur-last;
    if (d>worst) worst=d;
    last=cur;
    latency=worst;
  }
};
class latency_reset : public action {
public:
  latency_monitor *m;
  latency_reset(latency_monitor *m_) :m(m_) {}
  void loop() { m->worst=0; }
};

REGISTER_TABULA_DEVICE(latency,"",
  latency_monitor *m=new latency_monitor();
  actions_1ms.add(m);
  actions_1s.add(new latency_reset(m));
)

// Neato XV-11 laser distance sensor
#include "neato_serial.h"

class neato : public action {
public:
	NeatoLDS<Stream> n;
	tabula_sensor<NeatoLDSbatch> batch;
	int neatoMotorPin;
	neato(Stream &s,int motorPin_)
		:n(s), neatoMotorPin(motorPin_)
	{

	}

	void loop() {
		// Incoming comms
		int leash=1000; // bound maximum latency
		while (n.read()) { if (--leash<0) break; }

		// Outgoing comms--copy over the last batch
		batch=n.lastBatch;

		// Motor control
		pinMode(neatoMotorPin,OUTPUT);
		if (pc_connected) {
			if (n.lastBatch.speed64>200.0) { // PWM
				int target=280; // target RPM
				int err=n.lastBatch.speed64/64-target;
				analogWrite(neatoMotorPin,255-constrain(err*10 + 30, 0, 255));
			} else { // simple hardcoded startup value
				analogWrite(neatoMotorPin,255);
			}
		} else { // PC not connected
			analogWrite(neatoMotorPin,0); // turn motor off
		}
	}
};

REGISTER_TABULA_DEVICE(neato,"SP",
	Stream *s=src.read_serial(115200);
	int motorPin=src.read_pin();
	neato *n=new neato(*s,motorPin);
	actions_1ms.add(n);
)
