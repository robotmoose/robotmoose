#ifndef ROOMBA_H
#define ROOMBA_H

#ifdef __AVR

	#include <Arduino.h>

	class roomba_serial_t
	{
		public:
			roomba_serial_t(const HardwareSerial& serial);
			int available();
			size_t write(const void* buffer,const size_t size);
			size_t read(void* buffer,const size_t size);

		private:
			HardwareSerial* serial_m;
	};

#else

	#include <cctype>
	#include <msl/serial.hpp>
	#include <msl/time.hpp>

	typedef msl::serial_t roomba_serial_t;

#endif

class roomba_t
{
	public:
		enum mode_t
		{
			PASSIVE,
			SAFE,
			FULL
		};

		enum parser_state_t
		{
			HEADER,
			SIZE,
			DATA,
			CHECKSUM
		};

		roomba_t(const roomba_serial_t& serial);

		void start();
		void stop();
		void reset();

		void update();

		void set_mode(const mode_t& mode);

		void drive(const int16_t left,const int16_t right);

		void set_led_check(const bool on);
		void set_led_dock(const bool on);
		void set_led_spot(const bool on);
		void set_led_debris(const bool on);
		void set_led_clean(const uint8_t color,const uint8_t brightness);
		void led_update();

		void set_7_segment(const uint8_t digits[4]);
		void set_7_segment(const char* text);

		void play_song(const uint8_t number);

		void set_receive_sensors(const bool on);

	private:
		roomba_serial_t* serial_m;
		uint8_t leds_m;
		uint8_t led_clean_color_m;
		uint8_t led_clean_brightness_m;
		uint8_t serial_size_m;
		uint8_t* serial_buffer_m;
		uint8_t serial_pointer_m;
		parser_state_t serial_state_m;
};

#endif
