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
