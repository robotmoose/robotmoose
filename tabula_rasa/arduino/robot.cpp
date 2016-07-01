#include "robot.h"

battery_t::battery_t(const uint16_t cell_count):cell_count(cell_count),
	cells((float*)malloc(sizeof(float)*cell_count))
{
	for(uint16_t ii=0;ii<cell_count;++ii)
		cells[ii]=0.0;
}

battery_t::~battery_t()
{
	free(cells);
}
