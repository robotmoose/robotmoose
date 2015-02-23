#include <iostream>
#include <string>
#include <msl/serial.hpp>
#include <msl/time.hpp>
#include "arduino/roomba.h"

struct led_t
{
	uint8_t id;
	uint8_t bitfield;
	uint8_t color;
	uint8_t intensity;
};

int main()
{
	std::string serial_port="/dev/ttyUSB0";
	size_t serial_baud=115200;

	while(true)
	{
		msl::serial_t serial(serial_port,serial_baud);
		serial.open();

		if(serial.good())
		{
			roomba_t roomba(serial);

			std::cout<<":)"<<std::endl;

			//Setup
			roomba.start();
			roomba.set_mode(roomba_t::FULL);

			//Set Some LEDs
			roomba.set_led_check(false);
			roomba.set_led_dock(false);
			roomba.set_led_spot(false);
			roomba.set_led_debris(false);
			roomba.set_led_clean(0,0);
			roomba.set_receive_sensors(true);
			roomba.drive(0,0);
			roomba.set_7_segment("----");

			while(serial.good())
			{
				roomba.update();

				auto sensors=roomba.get_sensors();

				if((int)sensors.mode==3)
				{
					//roomba.set_led_clean(255,255);
					roomba.set_7_segment("F   ");
				}
				else if((int)sensors.mode==2)
				{
					//roomba.set_led_clean(128,255);
					roomba.set_7_segment("S   ");
				}
				else
				{
					//roomba.set_led_clean(0,255);
					roomba.set_7_segment("OFF");
				}

				roomba.led_update();
				msl::delay_ms(1);
			}

			serial.close();
		}

		std::cout<<":("<<std::endl;

		msl::delay_ms(1);
	}

	return 0;
}