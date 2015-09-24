#ifndef ROOMBA_H
#define ROOMBA_H

#if(defined(__AVR))

	#include <Arduino.h>

	typedef Stream roomba_serial_t;
#else

	#include <cctype>
	#include <cstring>
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
		
		struct battery_t
		{
			uint8_t state; // charging/discharging
			int8_t temperature; // deg C
			// uint16_t voltage; // millivolts
			uint16_t charge; // milliamp-hours current charge state
			// uint16_t capacity; // milliamp-hours maximum charge
		};
		struct encoder_t
		{
			uint16_t L; // left wheel
			uint16_t R; // right wheel
		};

		struct sensor_t
		{
			uint8_t mode;
			uint8_t bumper;
		
		// Battery charge state
			battery_t battery;
		
		// Wheel encoders
			encoder_t encoder;
		
		// Downward-facing light sensors:
			uint8_t floor[4];
		
		// Forward-facing reflectance sensors:
			uint8_t light[6];
			uint8_t light_field;
		
		// User interface buttons:
			uint8_t buttons;
		};
		enum button_t {
			BUTTON_CLEAN=1<<0, // AKA power
			BUTTON_SPOT=1<<1,
			BUTTON_DOCK=1<<2,
			BUTTON_MINUTE=1<<3,
			BUTTON_HOUR=1<<4,
			BUTTON_DAY=1<<5,
			BUTTON_SCHEDULE=1<<6,
			BUTTON_CLOCK=1<<7
		};

		roomba_t(roomba_serial_t& serial);
		
		void setup(int BRC_pin);

		void start();
		void stop();
		void reset();

		void update();

		void set_mode(const mode_t& mode);

		//void drive(const int16_t left,const int16_t right);
		void drive(const int16_t velocity, const int16_t radius);

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

		const sensor_t &get_sensors() const;

		void dump_sensors() const;

	private:
		roomba_serial_t* serial_m;
		uint8_t leds_m;
		uint8_t led_clean_color_m;
		uint8_t led_clean_brightness_m;
		uint8_t serial_size_m;
		uint8_t* serial_buffer_m;
		uint8_t serial_pointer_m;
		parser_state_t serial_state_m;
		sensor_t sensor_packet_m;

		bool parse_sensor_packet_m();
};

// The web front end needs a sensor name without colons::
typedef roomba_t::sensor_t roomba_sensor_t;

#endif
