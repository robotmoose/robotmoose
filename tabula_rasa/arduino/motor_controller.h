#ifndef MOTOR_CONTROLLER
#define MOTOR_CONTROLLER

#include <Arduino.h>

class motor_controller_t
{
	public:
		virtual void setup()=0;
		virtual void drive(const int16_t left,const int16_t right)=0;
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
		sabertooth_controller_t(const int16_t serial_pin_RX,const int16_t serial_pin_TX, const int16_t baud);
		void setup();
		void drive(const int16_t left, const int16_t right);

	private:
		uint16_t serial_pin_RX;
		uint16_t serial_pin_TX;
		uint16_t baud;
};

class create2_controller_t:public motor_controller_t
{
	public:
		create2_controller_t(const int16_t serial_pin_RX,const int16_t serial_pin_TX, const int16_t baud);
		void setup();
		void drive(const int16_t left,const int16_t right);

	private:
		uint16_t serial_pin_RX;
		uint16_t serial_pin_TX;
		uint16_t baud;
};

#endif
