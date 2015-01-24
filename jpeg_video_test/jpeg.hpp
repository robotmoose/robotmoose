#ifndef JPEG_HPP
#define JPEG_HPP

#include <cstdint>
#include <cstdio>
#include <jpeglib.h>
#include <string>
#include <vector>

class jpeg_t
{
	public:
		jpeg_t(const uint8_t* buffer,const size_t width,const size_t height,const size_t components,const size_t quality);
		jpeg_t(const std::vector<uint8_t>& buffer,const size_t width,const size_t height,const size_t components,const size_t quality);
		jpeg_t(const jpeg_t& copy);
		~jpeg_t();
		jpeg_t& operator=(const jpeg_t& copy);

		void clear();

		uint8_t* begin();
		const uint8_t* begin() const;

		uint8_t* end();
		const uint8_t* end() const;

		uint8_t& operator[](size_t index);
		const uint8_t& operator[](const size_t index) const;

		size_t size() const;
		size_t width() const;
		size_t height() const;
		size_t components() const;
		size_t quality() const;

	private:
		void compress_m(const uint8_t* buffer,const size_t width,const size_t height,const size_t components,const size_t quality);

		size_t size_m;
		uint8_t* data_m;
		size_t width_m;
		size_t height_m;
		size_t components_m;
		size_t quality_m;
};

#endif