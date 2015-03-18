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
<<<<<<< HEAD
	digitalWrite(left_pwm_pin_m,0);
	digitalWrite(right_pwm_pin_m,0);
	analogWrite(left_pwm_pin_m,left_pwm);
	analogWrite(right_pwm_pin_m,right_pwm);
}

void sabertooth_controller_t::setup()
{
	/* Check serial_number and match it to the corresponding Serial port 
      Probably handle 0 in the future
	*/
	if(serial_number = 1)
		Serial1.begin(baud);
	else if(serial_number = 2)
		Serial2.begin(baud);
	else if(serial_number = 3)
		Serial3.begin(baud);
}

void sabertooth_controller_t::drive(const int16_t left,const int16_t right)
{
	robot.drive.left = left + 64;
	robot.drive.right = right + 64;
	
	
}

	
	
	
=======
	for(int16_t ii=0;ii<2;++ii)
	{
		digitalWrite(left_pwms_m[ii],LOW);
		digitalWrite(right_pwms_m[ii],LOW);
	}

	analogWrite(left_pwms_m[left>=0],abs(left));
	analogWrite(right_pwms_m[right>=0],abs(right));
}
>>>>>>> 231b11edb9fb5b8932233e0560f4bc7e3558e428
