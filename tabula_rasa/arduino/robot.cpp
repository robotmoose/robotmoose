#include "robot.h"

battery_t::battery_t(const uint16_t cell_count):cell_count(cell_count),
	cells((float*)malloc(sizeof(float)*cell_count))
{}

battery_t::battery_t(const battery_t& copy)
{
	delete[] cells;
	cell_count=copy.cell_count;

	for(uint16_t ii=0;ii<cell_count;++ii)
		cells[ii]=copy.cells[ii];
}

battery_t::~battery_t()
{
	delete[] cells;
}

battery_t& battery_t::operator=(const battery_t& copy)
{
	if(this!=&copy)
	{
		delete[] cells;
		cell_count=copy.cell_count;

		for(uint16_t ii=0;ii<cell_count;++ii)
			cells[ii]=copy.cells[ii];
	}

	return *this;
}