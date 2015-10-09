/*
Clayton Auld,
September 11, 2015
clayauld@gmailcom
(Public Domain)

Code to test magnetic hall effect encoders using Melexis US5781 SMD
Pinout is as follows:
    Pin 1 (left):   Vcc to Arduino +5v or +3.3V
    Pin 2 (right):  Vout to Arduino interrupt pin
    Pin 3 (bottom): Gnd to Arduino Gnd

Some code borrowed and modified from http://yameb.blogspot.com/2012/11/quadrature-encoders-in-arduino-done.html

*/

#include <digitalWriteFast.h>
#include "tabula_control.h"
#include "tabula_config.h"

class layla_encoder : public action {
public:
	int hall_pin; // Hall effect sensor pin
	tabula_sensor<unsigned char> count; // encoder counts

	layla_encoder() {
		count = 0; // Encoder not connected
	}

	virtual void setup() {
		pinMode(hall_pin,INPUT_PULLUP); // enable pullup and input
    attachInterrupt(digitalPinToInterrupt(hall_pin), IncrementEncoder, CHANGE);
	}

	virtual void IncrementEncoder() {
		count = count + 1; // Increment encoder count
	}
};

REGISTER_TABULA_DEVICE(encoder,"P",
	int pin=src.read_pin();
	layla_encoder *device=new layla_encoder();
	device->hall_pin=pin;
	actions_1s.add(device);
)
