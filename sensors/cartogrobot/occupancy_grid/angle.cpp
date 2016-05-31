/*
 * angle.cpp
 * Author: Aven Bross, Max Hesser-Knoll
 * 
 * Simple angle class
*/

#include "angle.h"

angle::angle(): radians(0.0) {}

angle::angle(double radians): radians(radians) {
	if(std::abs(radians) >= 2*M_PI){
		radians = std::fmod(radians, 2.0*M_PI);
	}
	if(radians < 0){
		radians += 2*M_PI;
	}
}

angle & angle::operator+=(const angle & other){
	radians += other.radians;
	if(radians >= 2*M_PI){
		radians -= 2*M_PI;
	}
	return *this;
}

angle & angle::operator-=(const angle & other){
	radians -= other.radians;
	if(radians < 0){
		radians += 2*M_PI;
	}
	return *this;
}

angle & angle::operator*=(const angle & other){
	radians *= other.radians;
	if(std::abs(radians) >= 2*M_PI){
		radians = std::fmod(radians, 2.0*M_PI);
	}
	if(radians < 0){
		radians += 2*M_PI;
	}
	return *this;
}

angle & angle::operator/=(const angle & other){
	radians /= other.radians;
	if(std::abs(radians) >= 2*M_PI){
		radians = std::fmod(radians, 2.0*M_PI);
	}
	if(radians < 0){
		radians += 2*M_PI;
	}
	return *this;
}