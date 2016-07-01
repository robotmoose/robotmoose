/**
 A 2-float vector type suspiciously similar to GLSL's "vec2".
 
 Orion Sky Lawlor, olawlor@acm.org, 2008-03-05 (Public Domain)
*/
#ifndef __OSL_VEC2_H
#define __OSL_VEC2_H

#include "vector2d.h"
typedef osl::Vector2f vec2;
inline vec2 normalize(const vec2 &v) {return v/v.mag();}
inline float dot(const vec2 &a,const vec2 &b) {return a.dot(b);}
inline float length(const vec2 &a) {return a.mag();}
inline vec2 clamp(const vec2 &a,double lo,double hi) {
	vec2 ret=a;
	for (int axis=0;axis<2;axis++) {
		if (ret[axis]<lo) ret[axis]=lo;
		if (ret[axis]>hi) ret[axis]=hi;
	}
	return ret;
}

#endif
