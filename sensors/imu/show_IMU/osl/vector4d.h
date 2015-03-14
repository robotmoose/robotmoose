/*
Orion's Standard Library
Orion Sky Lawlor, 2005/1/30
NAME:		osl/vector4d.h

DESCRIPTION:	C++ 3-Dimentional homogenous vector library

3D vectors with a "w" field: 1 for position vectors,
0 for direction vectors.
*/
#ifndef __OSL_VECTOR4D_H
#define __OSL_VECTOR4D_H

#include <math.h> /* for sqrt */

#ifndef __OSL_VECTOR3D_H
#   include "osl/vector3d.h"
#endif

#ifdef __CHARMC__ /* for migration */
#  include "pup.h"
#endif

namespace osl {

/**
  A 4-component vector, often used to represent homogenous
  coordinates in space.  For a location vector, w==1; for
  a direction vector, w==0.
*/
template <class real>
class Vector4dT {
	typedef Vector4dT<real> vec;
public:
	real x,y,z,w;
	Vector4dT(void) {}//Default consructor
	/// Simple 1-value constructors
	explicit Vector4dT(int init) {x=y=z=w=(real)init;}
	explicit Vector4dT(float init) {x=y=z=w=(real)init;}
	explicit Vector4dT(double init) {x=y=z=w=(real)init;}
	/// 3-value constructor
	Vector4dT(const real Nx,const real Ny,const real Nz,const real Nw) {x=Nx;y=Ny;z=Nz;w=Nw;}
	/// real array constructor
	Vector4dT(const double *arr) {x=arr[0];y=arr[1];z=arr[2];w=arr[3];}
	Vector4dT(const float *arr) {x=arr[0];y=arr[1];z=arr[2];w=arr[3];}

	/// Constructors from other types of Vector:
	template <class Z>
	Vector4dT(const Vector3dT<Z> &src,real newW=1.0) 
	  {x=(real)src.x; y=(real)src.y; z=(real)src.z; w=newW; }
	template <class Z>
	Vector4dT(const Vector4dT<Z> &src) 
	  {x=(real)src.x; y=(real)src.y; z=(real)src.z; w=(real)src.w; }

	/// Copy constructor & assignment operator by default
	
	/// This lets you typecast a vector to a real array
	operator real *() {return (real *)&x;}
	operator const real *() const {return (const real *)&x;}

/// Basic mathematical operators	
	int operator==(const vec &b) const {return (x==b.x)&&(y==b.y)&&(z==b.z)&&(w==b.w);}
	int operator!=(const vec &b) const {return (x!=b.x)||(y!=b.y)||(z!=b.z)||(w!=b.w);}
	vec operator+(const vec &b) const {return vec(x+b.x,y+b.y,z+b.z,w+b.w);}
	vec operator-(const vec &b) const {return vec(x-b.x,y-b.y,z-b.z,w-b.w);}
	vec operator*(const real scale) const 
		{return vec(x*scale,y*scale,z*scale,w*scale);}
	vec operator/(const real &div) const
		{real scale=1.0/div;return vec(x*scale,y*scale,z*scale,w*scale);}
	vec operator-(void) const {return vec(-x,-y,-z,-w);}
	void operator+=(const vec &b) {x+=b.x;y+=b.y;z+=b.z;w+=b.w;}
	void operator-=(const vec &b) {x-=b.x;y-=b.y;z-=b.z;w-=b.w;}
	void operator*=(const real scale) {x*=scale;y*=scale;z*=scale;w*=scale;}
	void operator/=(const real div) {real scale=1.0/div;x*=scale;y*=scale;z*=scale;w*=scale;}

/// Vector-specific operations (all inherited from vector3d)
	/// Return the square of the magnitude of this vector
	real magSqr(void) const {return x*x+y*y+z*z+w*w;}
	/// Return the magnitude (length) of this vector
	real mag(void) const {return sqrt(magSqr());}
	
	/// Return the square of the distance to the vector b
	real distSqr(const vec &b) const 
		{return (x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)+(z-b.z)*(z-b.z)+(w-b.w)*(w-b.w);}
	/// Return the distance to the vector b
	real dist(const vec &b) const {return sqrt(distSqr(b));}
	
	/// Return the dot product of this vector and b
	real dot(const vec &b) const {return x*b.x+y*b.y+z*b.z+w*b.w;}
	/// Return the cosine of the angle between this vector and b
	real cosAng(const vec &b) const {return dot(b)/(mag()*b.mag());}
	
	/// Return the "direction" (unit vector) of this vector
	vec dir(void) const {return (*this)/mag();}
	/// Make this vector have unit length
	void normalize(void) { *this=this->dir();}
	
#ifdef __CK_PUP_H
	void pup(PUP::er &p) {p|x;p|y;p|z;p|w;}
#endif
};

/** Utility wrapper routines */
template<class real>
inline real dist(const Vector4dT<real> &a,const Vector4dT<real> &b)
	{ return a.dist(b); }

template<class real>
inline real dot(const Vector4dT<real> &a,const Vector4dT<real> &b)
	{ return a.dot(b); }

template<class real>
inline Vector4dT<real> cross(const Vector4dT<real> &a,const Vector4dT<real> &b)
	{ return a.cross(b); }

/** Allows "3.0*vec" to work. */
template <class scalar_t,class real>
inline Vector4dT<real> operator*(const scalar_t scale,const Vector4dT<real> &v)
	{return Vector4dT<real>(v.x*scale,v.y*scale,v.z*scale,v.w*scale);}

typedef Vector4dT<double> Vector4d;
typedef Vector4dT<float> Vector4f;
typedef Vector4dT<int> Vector4i;

}; //end namespace osl

#endif //__OSL_VECTOR4D_H


