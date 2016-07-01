#ifndef MSL_C11_MATRIX_HPP
#define MSL_C11_MATRIX_HPP

#include <stdexcept>
#include <cmath>
#include "vector.hpp"

namespace msl
{
	template<typename T> class matrix:public msl::vector<msl::vector<T>>
	{
		public:
			matrix(const size_t rows,const size_t cols):
				msl::vector<msl::vector<T>>(rows,msl::vector<T>(cols)),
				rows_m(rows),cols_m(cols)
			{}

			size_t size() const
			{
				return rows()*cols();
			}

			size_t rows() const
			{
				return rows_m;
			}

			size_t cols() const
			{
				return cols_m;
			}

		private:
			size_t rows_m;
			size_t cols_m;
	};

	template<typename T> class mat2:public matrix<T>
	{
		public:
			mat2(const msl::vec2<T>& r0,const msl::vec2<T>& r1):matrix<T>(2,2)
			{
				this->operator[](0)=r0;
				this->operator[](1)=r1;
			}
	};

	template<typename T> class mat3:public matrix<T>
	{
		public:
			mat3(const msl::vec3<T>& r0,const msl::vec3<T>& r1,const msl::vec3<T>& r2):matrix<T>(3,3)
			{
				this->operator[](0)=r0;
				this->operator[](1)=r1;
				this->operator[](2)=r2;
			}
	};

	template<typename T> class mat4:public matrix<T>
	{
		public:
			mat4(const msl::vec4<T>& r0,const msl::vec4<T>& r1,const msl::vec4<T>& r2,
				const msl::vec4<T>& r3):matrix<T>(4,4)
			{
				this->operator[](0)=r0;
				this->operator[](1)=r1;
				this->operator[](2)=r2;
				this->operator[](3)=r3;
			}
	};

	typedef mat2<int> mat2i;
	typedef mat2<float> mat2f;
	typedef mat2<double> mat2d;

	typedef mat3<int> mat3i;
	typedef mat3<float> mat3f;
	typedef mat3<double> mat3d;

	typedef mat4<int> mat4i;
	typedef mat4<float> mat4f;
	typedef mat4<double> mat4d;

	template<typename T> matrix<T> append(const matrix<T>& lhs,const matrix<T>& rhs)
	{
		if(lhs.rows()!=rhs.rows())
			throw std::runtime_error("matrix::append() - matrices must have the same number of rows.");

		matrix<T> app(lhs.rows(),lhs.cols()+rhs.cols());

		for(size_t row=0;row<app.rows();++row)
		{
			for(size_t ii=0;ii<lhs.cols();++ii)
				app[row][ii]=lhs[row][ii];
			for(size_t ii=0;ii<rhs.cols();++ii)
				app[row][lhs.cols()+ii]=rhs[row][ii];
		}

		return app;
	}

	template<typename T> matrix<T> cut(const matrix<T>& mat,const size_t start,const size_t cols)
	{
		matrix<T> ret(mat.rows(),cols);

		for(size_t row=0;row<ret.rows();++row)
			for(size_t col=0;col<cols;++col)
				ret[row][col]=mat[row][col+start];

		return ret;
	}

	template<typename T> T determinant(matrix<T> mat)
	{
		if(mat.rows()!=mat.cols())
			throw std::runtime_error("matrix::append() - matrix must have the same number of rows as cols.");

		T determinant=0;

		if(mat.rows()>0)
		{
			for(ssize_t ii=0;ii<mat.rows();++ii)
			{
				if(ii==0)
					determinant=mat[ii][ii];
				else
					determinant*=mat[ii][ii];

				if(mat[ii][ii]!=0)
				{
					mat[ii]=mat[ii]/mat[ii][ii];

					for(size_t jj=ii+1;jj<mat.rows();++jj)
						mat[jj]=mat[jj]-mat[ii]*mat[jj][ii];
				}
			}

			for(ssize_t ii=0;ii<mat.rows();++ii)
				determinant*=mat[ii][ii];
		}

		return determinant;
	}

	template<typename T> matrix<T> identity(const size_t rows,const size_t cols)
	{
		matrix<T> ident(rows,cols);

		for(ssize_t ii=0;ii<std::min(cols,rows);++ii)
			ident[ii][ii]=1;

		return ident;
	}

	template<typename T> matrix<T> inverse(const matrix<T>& mat)
	{
		if(mat.rows()!=mat.cols())
			throw std::runtime_error("matrix::append() - matrix must have the same number of rows as cols.");

		if(determinant(mat)==0)
			return matrix<T>(mat.rows(),mat.cols());

		return cut(solve(append<T>(mat,identity<T>(mat.rows(),mat.cols()))),mat.cols(),mat.cols());
	}

	template<typename T> matrix<T> solve(matrix<T> mat)
	{
		size_t min=std::min(mat.cols(),mat.rows());

		if(min>0)
		{
			for(ssize_t ii=0;ii<min;++ii)
			{
				if(mat[ii][ii]!=0)
				{
					mat[ii]=mat[ii]/mat[ii][ii];

					for(size_t jj=ii+1;jj<min;++jj)
						mat[jj]=mat[jj]-mat[ii]*mat[jj][ii];
				}
			}

			for(ssize_t ii=min-2;ii>=0;--ii)
				for(ssize_t jj=1;jj<min-ii;++jj)
					mat[ii]=mat[ii]-mat[ii+jj]*mat[ii][ii+jj];
		}

		return mat;
	}

	template<typename T> matrix<T> transpose(const matrix<T>& mat)
	{
		matrix<T> trans(mat.cols(),mat.rows());

		for(size_t yy=0;yy<mat.rows();++yy)
			for(size_t xx=0;xx<mat.cols();++xx)
				trans[xx][yy]=mat[yy][xx];

		return trans;
	}

	template<typename T> matrix<T> operator-(matrix<T> mat)
	{
		for(auto& row:mat)
			row=-row;

		return mat;
	}

	template<typename T> matrix<T> operator+(matrix<T> lhs,const matrix<T>& rhs)
	{
		if(lhs.rows()!=rhs.rows())
			throw std::runtime_error("matrix::operator+() - matrices must have the same number of rows.");
		if(lhs.cols()!=rhs.cols())
			throw std::runtime_error("matrix::operator+() - matrices must have the same number of cols.");

		for(size_t row=0;row<lhs.rows();++row)
			lhs[row]=lhs[row]+rhs[row];

		return lhs;
	}

	template<typename T> matrix<T> operator+=(matrix<T>& lhs,const matrix<T>& rhs)
	{
		return lhs=lhs+rhs;
	}

	template<typename T> matrix<T> operator-(matrix<T> lhs,const matrix<T>& rhs)
	{
		return lhs+(-rhs);
	}

	template<typename T> matrix<T> operator-=(matrix<T>& lhs,const matrix<T>& rhs)
	{
		return lhs=lhs-rhs;
	}

	template<typename T> matrix<T> operator*(matrix<T> lhs,const T& rhs)
	{
		for(size_t row=0;row<lhs.rows();++row)
			lhs[row]=lhs[row]*rhs;

		return lhs;
	}

	template<typename T> matrix<T> operator*(const T& lhs,matrix<T> rhs)
	{
		return rhs*lhs;
	}

	template<typename T> matrix<T> operator*=(matrix<T>& lhs,const T& rhs)
	{
		return lhs=lhs*rhs;
	}

	template<typename T> matrix<T> operator/(matrix<T> lhs,const T& rhs)
	{
		for(size_t row=0;row<lhs.rows();++row)
			lhs[row]=lhs[row]/rhs;

		return lhs;
	}

	template<typename T> matrix<T> operator/(const T& lhs,matrix<T> rhs)
	{
		return rhs/lhs;
	}

	template<typename T> matrix<T> operator/=(matrix<T>& lhs,const T& rhs)
	{
		return lhs=lhs/rhs;
	}

	template<typename T> matrix<T> operator*(const matrix<T>& lhs,const matrix<T>& rhs)
	{
		matrix<T> product(lhs.rows(),rhs.cols());

		for(size_t row=0;row<product.rows();++row)
			for(size_t col=0;col<product.cols();++col)
				product[row][col]=dot<T>(lhs[row],transpose<T>(rhs)[col]);

		return product;
	}

	template<typename T> matrix<T> operator*=(matrix<T>& lhs,const matrix<T>& rhs)
	{
		return lhs=lhs*rhs;
	}

	template<typename T> matrix<T> mult(const matrix<T>& lhs,const matrix<T>& rhs)
	{
		if(lhs.cols()!=rhs.rows())
			throw std::runtime_error("matrix::mult() - lhs cols must be equal to rhs rows.");

		return lhs*rhs;
	}

	template<typename T> bool operator==(const matrix<T>& lhs,const matrix<T>& rhs)
	{
		if(lhs.size()!=rhs.size())
			return false;

		for(size_t ii=0;ii<lhs.rows();++ii)
			if(lhs[ii]!=rhs[ii])
				return false;

		return true;
	}

	template<typename T> bool operator!=(const matrix<T>& lhs,const matrix<T>& rhs)
	{
		return !(lhs==rhs);
	}
};

#endif