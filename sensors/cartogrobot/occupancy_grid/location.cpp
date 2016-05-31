/*
 * location.cpp
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 11/29/2015
 * 
 * Description:
 * Robot location representation
*/

#include "location.h"

/* 
 * map_location
 * Stores a point as cartesian coordinates and a normal vector as angle
*/

// Defualt ctor
map_location::map_location(): x(0.0), y(0.0), direction(0.0) {}

// Constructs MapPoint with the given parameters
map_location::map_location(double x, double y, const angle & dir): x(x), y(y), direction(dir) {}

// Constructs MapPoint from local polar coordinates and MapPoint for their origin
map_location::map_location(double r, const angle & dir): direction(dir) {
	// Convert to cartesian and add vectors
	x = std::cos(direction) * r;
	y = std::sin(direction) * r;
}


// Constructs MapPoint from local polar coordinates and MapPoint for their origin
map_location::map_location(const map_location & location, double r, const angle & dir){
	// Correct for robot orientation
	direction = dir + location.get_direction();
	
	// Convert to cartesian and add vectors
	x = std::cos(direction) * r + location.get_x();
	y = std::sin(direction) * r + location.get_y();
}

// Self modifying addition
map_location & map_location::operator+=(const map_location & other){
	this->x += other.get_x();
	this->x += other.get_y();
	this->direction += other.get_direction();
	
	return *this;
}

// Non modifying addition
map_location operator+(const map_location & p1, const map_location & p2){
	return map_location(p1) += p2;
}

// Accessor for x coordinate
double map_location::get_x() const{
	return x;
}

// Accessor for y coordinate
double map_location::get_y() const{
	return y;
}

// Accessor for normal angle
const angle & map_location::get_direction() const{
	return direction;
}
