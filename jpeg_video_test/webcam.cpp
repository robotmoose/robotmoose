//Required Libraries:
//	opencv_core
//	opencv_highgui

#include "webcam.hpp"

#include <vector>

bool webcam_t::good() const
{
	return (capture_m.isOpened()&&!frame_m.empty());
}

void webcam_t::open(const size_t number)
{
	capture_m.open(number);
	update();
}

void webcam_t::close()
{
	capture_m.release();
}

void webcam_t::update()
{
	capture_m>>frame_m;
}

size_t webcam_t::width() const
{
	return frame_m.cols;
}

size_t webcam_t::height() const
{
	return frame_m.rows;
}

const void* webcam_t::data() const
{
	return frame_m.ptr();
}

std::string webcam_t::jpg(const size_t quality) const
{
	try
	{
		std::vector<unsigned char> buffer;
		std::vector<int> params;
		params.push_back(CV_IMWRITE_JPEG_QUALITY);
		params.push_back(quality);
		cv::imencode(".jpg",frame_m,buffer,params);
		return std::string((char*)buffer.data(),buffer.size());
	}
	catch(...)
	{
		return "";
	}
}