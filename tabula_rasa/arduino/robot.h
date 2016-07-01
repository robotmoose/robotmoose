#ifndef ROBOT_H
#define ROBOT_H

#if(defined(__AVR))
	#include <Arduino.h>
#else
	#include <cstdint>
	#include <cstdlib>
#endif

struct drive_t
{
	int16_t left;
	int16_t right;
};

//Make this a typedef...placeholder...
struct rotation_t
{
	float x;
	float y;
	float z;
};

//Make this a typedef...placeholder...
struct position_t
{
	float x;
	float y;
	float z;
};

//Placeholder...
class battery_t
{
	public:
		battery_t(const uint16_t cell_count=3);
		~battery_t();

		uint16_t cell_count;
		float* cells;

	private: // Don't copy these objects:
		battery_t(const battery_t& copy);
		battery_t& operator=(const battery_t& copy);
};

struct robot_t
{
	drive_t drive;
	rotation_t rotation;
	position_t position;
	battery_t battery;
};

#endif
