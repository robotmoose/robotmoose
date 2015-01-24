#include "jpeg.hpp"

jpeg_t::jpeg_t(const uint8_t* buffer,const size_t width,const size_t height,const size_t components,const size_t quality):
	size_m(0),data_m(nullptr),width_m(width),height_m(height),components_m(components),quality_m(quality)
{
	compress_m(buffer,width_m,height_m,components_m,quality_m);
}

jpeg_t::jpeg_t(const std::vector<uint8_t>& buffer,const size_t width,const size_t height,const size_t components,const size_t quality):
	size_m(0),data_m(nullptr),width_m(width),height_m(height),components_m(components),quality_m(quality)
{
	compress_m(buffer.data(),width_m,height_m,components_m,quality_m);
}

jpeg_t::jpeg_t(const jpeg_t& copy)
{
	clear();
	size_m=copy.size_m;
	data_m=new uint8_t[size_m];
	width_m=copy.width_m;
	height_m=copy.height_m;
	components_m=copy.components_m;
	quality_m=copy.quality_m;

	for(size_t ii=0;ii<size_m;++ii)
		data_m[ii]=copy.data_m[ii];
}

jpeg_t::~jpeg_t()
{
	clear();
}

jpeg_t& jpeg_t::operator=(const jpeg_t& copy)
{
	if(this!=&copy)
	{
		clear();
		size_m=copy.size_m;
		data_m=new uint8_t[size_m];

		for(size_t ii=0;ii<size_m;++ii)
			data_m[ii]=copy.data_m[ii];
	}

	return *this;
}

void jpeg_t::clear()
{
	size_m=0;
	free(data_m);
	data_m=nullptr;
	width_m=0;
	height_m=0;
	components_m=0;
	quality_m=1;
}

uint8_t* jpeg_t::begin()
{
	return data_m;
}

const uint8_t* jpeg_t::begin() const
{
	return data_m;
}

uint8_t* jpeg_t::end()
{
	return data_m+size_m;
}

const uint8_t* jpeg_t::end() const
{
	return data_m+size_m;
}

uint8_t& jpeg_t::operator[](size_t index)
{
	return data_m[index];
}
const uint8_t& jpeg_t::operator[](const size_t index) const
{
	return data_m[index];
}

size_t jpeg_t::size() const
{
	return size_m;
}

size_t jpeg_t::width() const
{
	return width_m;
}

size_t jpeg_t::height() const
{
	return height_m;
}

size_t jpeg_t::components() const
{
	return components_m;
}

size_t jpeg_t::quality() const
{
	return quality_m;
}

void jpeg_t::compress_m(const uint8_t* buffer,const size_t width,const size_t height,const size_t components,const size_t quality)
{
	clear();
	width_m=width;
	height_m=height;
	components_m=components;
	quality_m=quality;

	jpeg_compress_struct cinfo;

	jpeg_error_mgr jerr;
	cinfo.err=jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);

	jpeg_mem_dest(&cinfo,&data_m,&size_m);

	cinfo.image_width=width_m;
	cinfo.image_height=height_m;
	cinfo.input_components=components_m;
	cinfo.in_color_space=JCS_RGB;
	cinfo.dct_method=JDCT_FLOAT;

	jpeg_set_defaults(&cinfo);

	if(quality_m>100)
		quality_m=100;
	if(quality_m<1)
		quality_m=1;

	jpeg_set_quality(&cinfo,quality_m,true);

	jpeg_start_compress(&cinfo,true);

	while(cinfo.next_scanline<cinfo.image_height)
	{
		JSAMPROW row=(uint8_t*)&buffer[cinfo.next_scanline*cinfo.image_width*cinfo.input_components];
		jpeg_write_scanlines(&cinfo,&row,1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
}
