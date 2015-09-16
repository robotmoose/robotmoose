#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "action.h"
#include "tabula_control.h"
#include "roomba.h"

class motor_controller_t : public action
{
	public:
		virtual void drive(const int16_t left,const int16_t right)=0;

		/// By default a motor controller takes two speed commands
		tabula_command<int16_t> left, right;
		virtual void loop();
};

class bts_controller_t : public motor_controller_t
{
	public:
		bts_controller_t(uint16_t left_pwms[2],uint16_t right_pwms[2]);
		void drive(const int16_t left,const int16_t right);

	private:
		uint16_t left_pwms_m[2];
		uint16_t right_pwms_m[2];
};

class sabertooth_controller_t : public motor_controller_t
{
	public:
		sabertooth_controller_t(Stream& serial);
		void drive(const int16_t left,const int16_t right);

	protected:
		Stream* serial_m;
		virtual void send_motor_m(const uint8_t address,const uint8_t motor,const int16_t value)=0;
};

class sabertooth_v1_controller_t:public sabertooth_controller_t
{
	public:
		sabertooth_v1_controller_t(Stream& serial);

	private:
		void send_motor_m(const uint8_t address,const uint8_t motor,const int16_t value);
};

class sabertooth_v2_controller_t:public sabertooth_controller_t
{
	public:
		sabertooth_v2_controller_t(Stream& serial);

	private:
		void send_motor_m(const uint8_t address,const uint8_t motor,const int16_t value);
};

#endif
