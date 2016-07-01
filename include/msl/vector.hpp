#ifndef MSL_C11_VECTOR_HPP
#define MSL_C11_VECTOR_HPP

#include <stdexcept>
#include <cmath>
#include <vector>

namespace msl
{
	template<typename T> class vector
	{
		public:
			vector(const size_t size=0,const T& value=T()):data_m(size)
			{
				for(auto& ii:data_m)
					ii=value;
			}

			vector(const std::initializer_list<T> list):data_m(list)
			{}

			virtual ~vector()
			{}

			unsigned int size() const
			{
				return data_m.size();
			}

			T& operator[](const unsigned int index)
			{
				return data_m[index];
			}

			const T& operator[](const unsigned int index) const
			{
				return data_m[index];
			}

			T& at(const unsigned int index)
			{
				return data_m.at(index);
			}

			const T& at(const unsigned int index) const
			{
				return data_m.at(index);
			}

			T* begin()
			{
				if(size()==0)
					return nullptr;

				return &(this->operator[](0));
			}

			const T* begin() const
			{
				if(size()==0)
					return nullptr;

				return &(this->operator[](0));
			}

			T* end()
			{
				if(size()==0)
					return begin();

				return &(this->operator[](data_m.size()-1))+1;
			}

			const T* end() const
			{
				if(size()==0)
					return begin();

				return &(this->operator[](data_m.size()-1))+1;
			}

		private:
			std::vector<T> data_m;
	};

	template<typename T> class vec2:public vector<T>
	{
		public:
			vec2(const T& x,const T& y):vector<T>({x,y}),
				x(this->operator[](0)),y(this->operator[](1)),
				s(this->operator[](0)),t(this->operator[](1))
			{}

			vec2(const vec2<T>& copy):vector<T>(copy),
				x(this->operator[](0)),y(this->operator[](1)),
				s(this->operator[](0)),t(this->operator[](1))
			{}

			vec2(const vector<T>& copy):vector<T>(copy),
				x(this->operator[](0)),y(this->operator[](1)),
				s(this->operator[](0)),t(this->operator[](1))
			{}

			vec2<T>& operator=(const vec2<T>& copy)
			{
				if(this!=&copy)
					for(size_t ii=0;ii<this->size();++ii)
						this->operator[](ii)=copy[ii];

				return *this;
			}

			T& x;
			T& y;

			T& s;
			T& t;
	};

	template<typename T> class vec3:public vector<T>
	{
		public:
			vec3(const T& x,const T& y,const T& z):vector<T>({x,y,z}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2))
			{}

			vec3(const vec2<T>& vec,const T& z):vector<T>({vec.x,vec.y,z}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2))
			{}

			vec3(const T& x,const vec2<T>& vec):vector<T>({x,vec.x,vec.y}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2))
			{}

			vec3(const vec3<T>& copy):vector<T>(copy),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2))
			{}

			vec3(const vector<T>& copy):vector<T>(copy),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2))
			{}

			vec3<T>& operator=(const vec3<T>& copy)
			{
				if(this!=&copy)
					for(size_t ii=0;ii<this->size();++ii)
						this->operator[](ii)=copy[ii];

				return *this;
			}

			T& x;
			T& y;
			T& z;

			T& r;
			T& g;
			T& b;

			T& s;
			T& t;
			T& p;
	};

	template<typename T> class vec4:public vector<T>
	{
		public:
			vec4(const T& x,const T& y,const T& z,const T& w):vector<T>({x,y,z,w}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4(const vec3<T>& vec,const T& w):vector<T>({vec.x,vec.y,vec.z,w}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4(const T& x,const vec3<T>& vec):vector<T>({x,vec.x,vec.y,vec.z}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4(const vec2<T>& vecl,const vec2<T>& vecr):vector<T>({vecl.x,vecl.y,vecr.x,vecr.y}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4(const vec2<T>& vec,const T& z,const T& w):vector<T>({vec.x,vec.y,z,w}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4(const T& x,const vec2<T>& vec,const T& w):vector<T>({x,vec.x,vec.y,w}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4(const T& x,const T& y,const vec2<T>& vec):vector<T>({x,y,vec.x,vec.y}),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4(const vec4<T>& copy):vector<T>(copy),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4(const vector<T>& copy):vector<T>(copy),
				x(this->operator[](0)),y(this->operator[](1)),z(this->operator[](2)),w(this->operator[](3)),
				r(this->operator[](0)),g(this->operator[](1)),b(this->operator[](2)),a(this->operator[](3)),
				s(this->operator[](0)),t(this->operator[](1)),p(this->operator[](2)),q(this->operator[](3))
			{}

			vec4<T>& operator=(const vec4<T>& copy)
			{
				if(this!=&copy)
					for(size_t ii=0;ii<this->size();++ii)
						this->operator[](ii)=copy[ii];

				return *this;
			}

			T& x;
			T& y;
			T& z;
			T& w;

			T& r;
			T& g;
			T& b;
			T& a;

			T& s;
			T& t;
			T& p;
			T& q;
	};

	typedef vec2<int> vec2i;
	typedef vec2<float> vec2f;
	typedef vec2<double> vec2d;

	typedef vec3<int> vec3i;
	typedef vec3<float> vec3f;
	typedef vec3<double> vec3d;

	typedef vec4<int> vec4i;
	typedef vec4<float> vec4f;
	typedef vec4<double> vec4d;

	template<typename T> vector<T> operator-(vector<T> vec)
	{
		for(auto& ii:vec)
			ii=-ii;

		return vec;
	}

	template<typename T> vector<T> operator+(vector<T> lhs,const vector<T>& rhs)
	{
		if(lhs.size()!=rhs.size())
			throw std::runtime_error("vector::operator+() - vectors must be the same size.");

		for(unsigned int ii=0;ii<lhs.size();++ii)
			lhs[ii]=lhs[ii]+rhs[ii];

		return lhs;
	}

	template<typename T> vector<T> operator+=(vector<T>& lhs,const vector<T>& rhs)
	{
		return lhs=lhs+rhs;
	}

	template<typename T> vector<T> operator-(const vector<T>& lhs,const vector<T>& rhs)
	{
		if(lhs.size()!=rhs.size())
			throw std::runtime_error("vector::operator-() - vectors must be the same size.");

		return lhs+(-rhs);
	}

	template<typename T> vector<T> operator-=(vector<T>& lhs,const vector<T>& rhs)
	{
		return lhs-rhs;
	}

	template<typename T> vector<T> operator*(vector<T> lhs,const T& rhs)
	{
		for(auto& ii:lhs)
			ii*=rhs;

		return lhs;
	}

	template<typename T> vector<T> operator*(const T& lhs,vector<T> rhs)
	{
		return rhs*lhs;
	}

	template<typename T> vector<T> operator*=(vector<T>& lhs,const T& rhs)
	{
		return lhs=lhs*rhs;
	}

	template<typename T> vector<T> operator/(vector<T> lhs,const T& rhs)
	{
		for(auto& ii:lhs)
			ii/=rhs;

		return lhs;
	}

	template<typename T> vector<T> operator/(const T& lhs,vector<T> rhs)
	{
		return rhs/lhs;
	}

	template<typename T> vector<T> operator/=(vector<T>& lhs,const T& rhs)
	{
		return lhs=lhs/rhs;
	}

	template<typename T> T dot(const vector<T>& lhs,const vector<T>& rhs)
	{
		if(lhs.size()!=rhs.size())
			throw std::runtime_error("vector::dot() - vectors must be the same size.");

		T product=T();

		for(unsigned int ii=0;ii<lhs.size();++ii)
			product+=lhs[ii]*rhs[ii];

		return product;
	}

	template<typename T> vector<T> cross(const vector<T>& lhs,const vector<T>& rhs)
	{
		if(lhs.size()!=rhs.size())
			throw std::runtime_error("vector::cross() - vectors must be the same size.");
		if(lhs.size()!=3)
			throw std::runtime_error("vector::cross() - vectors must be of size 3.");

		return {lhs[1]*rhs[2]-lhs[2]*rhs[1],lhs[2]*rhs[0]-lhs[0]*rhs[2],lhs[0]*rhs[1]-lhs[1]*rhs[0]};
	}

	template<typename T> vector<T> normalize(const vector<T>& vec)
	{
		return vec/std::sqrt(dot<T>(vec,vec));
	}

	template<typename T> bool operator==(const vector<T>& lhs,const vector<T>& rhs)
	{
		if(lhs.size()!=rhs.size())
			return false;

		for(size_t ii=0;ii<lhs.size();++ii)
			if(lhs[ii]!=rhs[ii])
				return false;

		return true;
	}

	template<typename T> bool operator!=(const vector<T>& lhs,const vector<T>& rhs)
	{
		return !(lhs==rhs);
	}
};

#endif