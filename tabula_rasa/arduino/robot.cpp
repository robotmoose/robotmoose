#include "robot.h"

battery_t::battery_t(const uint16_t cell_count):cell_count(cell_count),
	cells((float*)malloc(sizeof(float)*cell_count))
{
	for(uint16_t ii=0;ii<cell_count;++ii)
		cells[ii]=0.0;
}

battery_t::battery_t(const battery_t& copy)
{
	free(cells);
	cell_count=copy.cell_count;

	for(uint16_t ii=0;ii<cell_count;++ii)
		cells[ii]=copy.cells[ii];
}

battery_t::~battery_t()
{
	free(cells);
}

battery_t& battery_t::operator=(const battery_t& copy)
{
	if(this!=&copy)
	{
		free(cells);
		cell_count=copy.cell_count;

		for(uint16_t ii=0;ii<cell_count;++ii)
			cells[ii]=copy.cells[ii];
	}

	return *this;
}

robot_t::robot_t(motor_controller_t* controller):controller_m(controller)
{}

void robot_t::loop()
{
}