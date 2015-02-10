/**
  General robot code, shared between the Arduino and the PC.

  Orion Sky Lawlor, lawlor@alaska.edu, 2014-03-23 (Public Domain)
*/
#ifndef __AURORA_ROBOTICS__ROBOT_H
#define __AURORA_ROBOTICS__ROBOT_H
#include <stdint.h> /* for uint16_t */

/// This bitfield convey's the robot's software status.
class robot_status_bits {
public:
	uint8_t stop:1; ///< EMERGENCY STOP engaged
	uint8_t arduino:1; ///< arduino is connected correctly
	uint8_t located:1; ///< robot thinks it knows where it is
	uint8_t autonomy:1; ///< full-autonomy mode is engaged
	uint8_t semiauto:1; ///< semiauto mode is engaged
};

/** This is the Arduino's AREF analog reference voltage.
  It's the scale factor that gives true voltage output,
  and should be measured from the AREF pin against Arduino ground. */
#define AD_AREF_voltage (4.90)

/** This scale factor converts an
    Arduino Analog/Digital Data Number (0-1023)
    to a real voltage, assuming direct feed-in. */
#define AD_DN2low_voltage (AD_AREF_voltage/(1023.0))

/** This scale factor converts an
    Arduino Analog/Digital Data Number (0-1023)
    to a real voltage, after the resistor divider scaling.
   The constant is a weird fudge factor--the resistances alone don't seem to add up.
*/
#define AD_DN2high_voltage ((AD_AREF_voltage+0.9)*2000.0/(348.0*1023.0))


/** This class contains all the robot's sensors, on Arduino, backend, or front end.
Raw sensor values go as bitfields, because many of them are 10-bit quantities:
	- Arduino A/D values are 10 bits each
	- Arena positions in cm are 9-10 bits each (arena is 378x738cm)
	- Blinky angle reports are about 9 bits each (500 samples per rotation)
*/
class robot_sensors_fast {
public:
	uint16_t uSound1:10; // ultra sound value 1-600ish 10 gives us 1024
	uint16_t uSound2: 10;
	uint16_t uSound3: 10;
	uint16_t padding : 2; // probly not needed


	uint16_t uSound4 : 10;
	uint16_t uSound5 : 10;
	uint16_t padding2 : 12;// probly not needed
};

class robot_sensors_slow {
public:
	uint16_t battery : 10; // raw A/D reading at top of battery stack (voltage = this*5*2000/384)
	uint16_t mining : 10; // raw A/D value of mining head drive voltage (stalled when much less than battery)
	uint16_t bucket : 10; // raw A/D value from dump bucket lift encoder

	uint16_t stalled : 1; ///< mining head seems to be stalled out
	uint16_t stop : 1; ///< EMERGENCY STOP button engaged

	uint16_t frontL : 10; // raw A/D value from front-left wheel deploy encoder
	uint16_t frontR : 10; // raw A/D value from front-right wheel deploy encoder
	uint16_t latency : 10; // Arduino control loop latency

	uint16_t backL : 1; ///< back left contact detected
	uint16_t backR : 1; ///< back right contact detected

	uint16_t bucketFill : 6; /// Bits for 6 infrared detectors
	uint16_t mineMoving : 10; /// leaky-bucket, incremented on mine motion (200 is max motion; 0 is stopped)
	uint16_t avgStall : 10; /// stall detection average
	uint16_t padding : 6;
};


/**
 This class contains a power setting for each of the robot's actuators.

 The ":7" makes each a 7-bit field, with values:
 	1 (reverse)
 	64 (stop)
 	127 (forward)
*/
class robot_power {
public:
	enum { drive_stop=64 };

	uint8_t left:7; // left drive wheels
	uint8_t high:1; // bool for LED system

	uint8_t right:7; // right drive wheels
	uint8_t backMode:1; //bool for LED demo

	uint8_t front:7; // For now front is servo1
	uint8_t frontUnused:1; // spare bit

	uint8_t mine:7; // servo2
	uint8_t mineMode:1; // if true, autonomously run mining head

	uint8_t dump:7; // servo3
	uint8_t dumpMode:1; // dock-and-dump mode

	robot_power() { stop(); }
	void stop(void) {
		left=right=front=mine=dump=drive_stop; // all-stop
		high=dumpMode=mineMode=0;
	}
};


class robot_led {
public:
	uint8_t red; // 0-255
	uint8_t green; // 0-255
	uint8_t blue; // 0-255

	uint8_t ledon : 1; // bool for led on
	uint8_t demo : 1; // bool for demo mode
	uint8_t unused : 6;
};

/**
 This class contains everything we currently know about the robot.
*/
class robot_current {
public:
	robot_status_bits status; ///< Current software status bits
	robot_sensors_fast f_sensors;  ///< fast responce sensor data
	robot_sensors_slow s_sensors;  ///< Current hardware sensor values
	robot_power power; // Current drive commands
	robot_led led;

	bool autonomous;
};

#endif

