/**
 A 4-float vector type suspiciously similar to GLSL's "vec4".
 
 Orion Sky Lawlor, olawlor@acm.org, 2006/09/12 (Public Domain)
*/
#ifndef __OSL_VEC4_H
#define __OSL_VEC4_H

#include "vector4d.h"
typedef osl::Vector4dT<float> vec4;
inline vec4 normalize(const vec4 &v) {return v.dir();}
inline float dot(const vec4 &a,const vec4 &b) {return a.dot(b);}
inline float length(const vec4 &a) {return a.mag();}
inline float clamp(float a,float lo,float hi) {
	if (a<lo) return lo;
	if (a>hi) return hi;
	return a;
}
inline vec4 clamp(const vec4 &a,double lo,double hi) {
	vec4 ret=a;
	for (int axis=0;axis<4;axis++) {
		if (ret[axis]<lo) ret[axis]=lo;
		if (ret[axis]>hi) ret[axis]=hi;
	}
	return ret;
}
/// Allow "vec*vec" to compile:
inline vec4 operator*(const vec4 &a,const vec4 &b) {return vec4(a.x*b.x,a.y*b.y,a.z*b.z,a.w*b.w);}

inline vec4 mix(const vec4 &a,const vec4 &b,float f) {return a+f*(b-a);}
inline vec4 min(const vec4 &a,const vec4 &b) {
	vec4 ret=a;
	for (int axis=0;axis<4;axis++) {
		if (ret[axis]>b[axis]) ret[axis]=b[axis];
	}
	return ret;
}
inline vec4 max(const vec4 &a,const vec4 &b) {
	vec4 ret=a;
	for (int axis=0;axis<4;axis++) {
		if (ret[axis]<b[axis]) ret[axis]=b[axis];
	}
	return ret;
}

#include "vector3d.h"
typedef osl::Vector3dT<float> vec3;
inline vec3 normalize(const vec3 &v) {return v.dir();}
inline float dot(const vec3 &a,const vec3 &b) {return a.dot(b);}
inline float length(const vec3 &a) {return a.mag();}
inline vec3 reflect(const vec3 &I,const vec3 &N) 
	{return I-2.0*dot(N,I)*N;}

/// Allow "vec*vec" to compile:
inline vec3 operator*(const vec3 &a,const vec3 &b) {return vec3(a.x*b.x,a.y*b.y,a.z*b.z);}

inline vec3 mix(const vec3 &a,const vec3 &b,float f) {return a+f*(b-a);}
inline vec3 min(const vec3 &a,const vec3 &b) {
	vec3 ret=a;
	for (int axis=0;axis<3;axis++) {
		if (ret[axis]>b[axis]) ret[axis]=b[axis];
	}
	return ret;
}
inline vec3 max(const vec3 &a,const vec3 &b) {
	vec3 ret=a;
	for (int axis=0;axis<3;axis++) {
		if (ret[axis]<b[axis]) ret[axis]=b[axis];
	}
	return ret;
}

/* that's all you need! */

#endif
