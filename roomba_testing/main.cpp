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
		msl::serial_t arduino(serial_port,serial_baud);
		arduino.open();

		if(arduino.good())
		{
			roomba_t roomba(arduino);

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
			roomba.set_7_segment(" -_-");
			roomba.set_receive_sensors(true);
			roomba.set_receive_sensors(true);
			roomba.set_receive_sensors(true);

			while(arduino.good())
			{
				roomba.update();
				//roomba.play_song(2);
				/*//Fade Big LED
				for(int ii=0;ii<256;++ii)
				{
					roomba.set_led_clean(0,ii);
					roomba.led_update();
					msl::delay_ms(2);
				}*/
				msl::delay_ms(1);
			}
		}

		std::cout<<":("<<std::endl;

		msl::delay_ms(1);
	}

	return 0;
}