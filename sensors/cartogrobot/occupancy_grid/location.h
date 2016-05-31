/*
 * location.h
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 11/29/2015
 * 
 * Description:
 * Robot location representation
*/

#ifndef __LOCATION_H__
#define __LOCATION_H__

#include<cmath>
#include<utility>
#include "angle.h"

// Stores a point as cartesian coordinates and a normal vector as angle
class map_location {
public:
    
    // Defualt ctor
    map_location();
    
    // Constructs MapPoint with the given parameters
    map_location(double x, double y, const angle & dir);
    
    // Constructs MapPoint from local polar coordinates and MapPoint for their origin
    map_location(double r, const angle & dir);
    
    // Constructs MapPoint from local polar coordinates and MapPoint for their origin
    map_location(const map_location & location, double r, const angle & direction);
    
    // Accessor for x coordinate
    double get_x() const;
    
    // Accessor for y coordinate
    double get_y() const;
    
    // Accessor for normal angle
    const angle & get_direction() const;
    
    // Self modifying addition
    map_location & operator+=(const map_location & other);  
    
private:
    // Cartesian coordinates
    double x, y;
    
    // Angle representing normal vector
    angle direction;
};

// Non modifying addition
map_location operator+(const map_location & p1, const map_location & p2);

#endif