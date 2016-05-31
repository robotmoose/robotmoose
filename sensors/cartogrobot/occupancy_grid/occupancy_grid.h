/*
 * occupancy_grid.h
 * Author: Aven Bross, Max Hesser-Knoll
 * 
 * Probabilistic occupancy grid
*/

#ifndef __OCCUPANCY_GRID_H__
#define __OCCUPANCY_GRID_H__

#include <vector>
#include <limits>
#include <iostream>
#include "location.h"


class occupancy_grid
{
	public:
		typedef double log_probability;
		typedef double probability;
		
		occupancy_grid(std::size_t size = 100, probability p_0 = 0.5, probability p_free = 0.2,
			probability p_occ = 0.8, log_probability l_max = 10.0);
		
		void update(const map_location & xt, const std::vector<double> & zt);
		
		probability operator()(std::size_t x, std::size_t y) const;
		
		std::size_t size() const;
	
	private:
		void range_sensor_update(const map_location & begin, const map_location & end);
	
		log_probability l_0;
		
		std::vector<std::vector<log_probability>> grid;
		
		log_probability l_occ, l_free;
		log_probability l_max;
};

#endif