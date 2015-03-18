#ifndef MOTOR_CONTROLLER
#define MOTOR_CONTROLLER

#include <Arduino.h>
#include "robot.h" // Remove this once a main program is created

class motor_controller_t
{
	public:
		virtual void setup()=0;
		virtual void drive(const int16_t left,const int16_t right)=0;

	private:
		robot_t robot;
};

class bts_controller_t:public motor_controller_t
{
	public:
		bts_controller_t(uint16_t left_pwms[2],uint16_t right_pwms[2]);
		void setup();
		void drive(const int16_t left,const int16_t right);

	private:
		uint16_t left_pwms_m[2];
		uint16_t right_pwms_m[2];
};

class sabertooth_controller_t:public motor_controller_t
{
	public:
		sabertooth_controller_t(Stream& serial,const uint32_t baud=9600);
		void setup();
		void drive(const int16_t left,const int16_t right);

	private:
		Stream* serial_m;
		uint32_t baud_m;

		void send_motor_m(const uint8_t address,const uint8_t motor,const int16_t value);
};

/*class create2_controller_t:public motor_controller_t
{
	public:
		create2_controller_t(const int16_t serial_number,const int16_t baud);
		void setup();
		void drive(const int16_t left,const int16_t right);

	private:
		uint16_t serial_number;
		uint16_t baud;
};*/

#endif
