/*
 * occupancy_grid.h
 * Author: Aven Bross, Max Hesser-Knoll
 * 
 * Probabilistic occupancy grid
*/

#include "occupancy_grid.h"

occupancy_grid::occupancy_grid(std::size_t size, probability p_0, probability p_free, probability p_occ,
    log_probability l_max) : l_max(l_max)
{
	l_0 = std::log(p_0 / (1 - p_0));
	l_occ = std::log(p_occ / (1 - p_occ));
	l_free = std::log(p_free / (1 - p_free));		// Just guessing here, might want to change
	
	grid = std::vector<std::vector<log_probability> >(size);
	
	for(auto & row : grid)
	{
		row = std::vector<log_probability>(size);
		
		for(auto & cell : row)
		{
			cell = l_0;
		}
	}
}

occupancy_grid::probability occupancy_grid::operator()(std::size_t x, std::size_t y) const
{
	return 1.0 - 1.0 / (1.0 + std::exp(grid[x][y]));
}

std::size_t occupancy_grid::size() const
{
	return grid.size();
}

void occupancy_grid::update(const map_location & xt, const std::vector<double> & zt)
{
	angle direction = 0.0;
	angle delta = 2.0 * M_PI / zt.size();	
	
	for(std::size_t i = 0; i < zt.size(); i++)
	{
		//std::cout << direction << ", ";
		if(zt[i] != 0.0)
		{
		    try{
			    range_sensor_update(xt, map_location(xt, zt[i], direction));
		    }
		    catch(std::exception & error)
		    {
		        std::cout<<"Error! "<<error.what()<<std::endl;
		    }
		}
		direction -= delta;
	}
	//std::cout << "\n";
}

void occupancy_grid::range_sensor_update(const map_location & begin, const map_location & end)
{
	double x0 = begin.get_x();
	double y0 = begin.get_y(); 
	
	double x1 = end.get_x();
	double y1 = end.get_y();
	
	double dx = fabs(x1 - x0);
    double dy = fabs(y1 - y0);

    std::size_t x = floor(x0);
    std::size_t y = floor(y0);

    std::size_t n = 1;
    int x_inc, y_inc;
    double error;

    // Compute x increment direction based on delta x
    if (dx == 0)
    {
        x_inc = 0;
        error = std::numeric_limits<double>::infinity();
    }
    else if (x1 > x0)
    {
        x_inc = 1;
        n += int(floor(x1)) - x;
        error = (floor(x0) + 1 - x0) * dy;
    }
    else
    {
        x_inc = -1;
        n += x - int(floor(x1));
        error = (x0 - floor(x0)) * dy;
    }

    // Compute y increment direction based on delta y
    if (dy == 0)
    {
        y_inc = 0;
        error -= std::numeric_limits<double>::infinity();
    }
    else if (y1 > y0)
    {
        y_inc = 1;
        n += int(floor(y1)) - y;
        error -= (floor(y0) + 1 - y0) * dx;
    }
    else
    {
        y_inc = -1;
        n += y - int(floor(y1));
        error -= (y0 - floor(y0)) * dx;
    }
    
    unsigned int maxn = n;

    // Update log probability for each grid cell on the rasterized line
    for (; n > 0; --n)
    {
        if(x > grid.size() || y > grid.size()) throw std::runtime_error("Sensor value out of bounds");
        // Update grid cells seen through by this vector
    	if(n > 1)
    	{
    		if(y > 0 && y < size() && x > 0 && x < size())
    		{
				// Saw through this square, not occupied
				grid[x][y] += l_free*(size())/(8*(size()+maxn-n)) - l_0;
				
				// Bound log probabilites
				if(grid[x][y] < -l_max) grid[x][y] = -l_max;
				if(grid[x][y] > l_max) grid[x][y] = l_max;
			}
    	}
    	// Update grid cell containing obstacle at the end of the vector
        else
        {
        	if(y > 0 && y < size() && x > 0 && x < size())
    		{
		    	// Object detected in this square, occupied
		    	grid[x][y] += l_occ*(size())/(8*(size()+maxn-n)) - l_0;
		    	
		    	// Bound log probabilites
		    	if(grid[x][y] < -l_max) grid[x][y] = -l_max;
		    	if(grid[x][y] > l_max) grid[x][y] = l_max;
	    	}
        }
        
        // If we are are "below" the line, update y
        if (error > 0)
        {
            y += y_inc;
            error -= dx;
        }
        // Otherwise, we are "above" the line, update x
        else
        {
            x += x_inc;
            error += dy;
        }
    }
}
