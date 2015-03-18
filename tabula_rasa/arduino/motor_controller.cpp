#include "motor_controller.h"

bts_controller_t::bts_controller_t(uint16_t left_pwms[2],uint16_t right_pwms[2])
{
	for(int16_t ii=0;ii<2;++ii)
	{
		left_pwms_m[ii]=left_pwms[ii];
		right_pwms_m[ii]=right_pwms[ii];
	}
}

void bts_controller_t::setup()
{
	for(int16_t ii=0;ii<2;++ii)
	{
		pinMode(left_pwms_m[ii],OUTPUT);
		pinMode(right_pwms_m[ii],OUTPUT);
		digitalWrite(left_pwms_m[ii],LOW);
		digitalWrite(right_pwms_m[ii],LOW);
	}
}

void bts_controller_t::drive(const int16_t left,const int16_t right)
{
	for(int16_t ii=0;ii<2;++ii)
	{
		digitalWrite(left_pwms_m[ii],LOW);
		digitalWrite(right_pwms_m[ii],LOW);
	}

	analogWrite(left_pwms_m[left>=0],abs(left));
	analogWrite(right_pwms_m[right>=0],abs(right));
}

sabertooth_controller_t::sabertooth_controller_t(HardwareSerial& serial,const uint32_t baud):
	serial_m(&serial),baud_m(baud)
{}

void sabertooth_controller_t::setup()
{
	serial_m->begin(baud_m);
}

void sabertooth_controller_t::drive(const int16_t left,const int16_t right)
{
	send_motor_m(128,6,left);
	send_motor_m(128,7,right);
}

void sabertooth_controller_t::send_motor_m(const uint8_t address,const uint8_t motor,const int16_t value)
{
	uint8_t value_raw=64+(value>>2);

	uint8_t data[4]=
	{
		address,
		motor,
		value_raw,
		(address+motor+value_raw)&127
	};

	serial_m->write(data,4);
}
