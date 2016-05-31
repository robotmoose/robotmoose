/*
 * angle.h
 * Author: Aven Bross, Max Hesser-Knoll
 * 
 * Simple angle class
*/

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif

#ifndef __ANGLE_H__
#define __ANGLE_H__

#include<cmath>

// Stores an angle in radians between 0 and 2pi
class angle {
public:
    angle();
    angle(double angle);
    
    // Self modifying arithmetic
    angle & operator+=(const angle & other);
    angle & operator-=(const angle & other);
    angle & operator*=(const angle & other);
    angle & operator/=(const angle & other);
    
    operator double() const{
        return radians;
    }
    
private:

    double radians;
};

#endif