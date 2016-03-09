//Required Libraries:
//	-lopencv_core -lopencv_highgui -lpthread

#include "webcam.hpp"

#include <chrono>
#include <thread>
#include <vector>

webcam_t::~webcam_t()
{
	close();
}

bool webcam_t::good() const
{
	return (capture_m.isOpened());
}

void webcam_t::open(const size_t number)
{
	close();
	capture_m.open(number);
	std::thread thread(&webcam_t::update_m,this);
	thread.detach();
}

void webcam_t::close()
{
	if(good())
		capture_m.release();
}

size_t webcam_t::width()
{
	frame_lock_m.lock();
	auto copy=frame_m;
	frame_lock_m.unlock();
	return copy.cols;
}

size_t webcam_t::height()
{
	frame_lock_m.lock();
	auto copy=frame_m;
	frame_lock_m.unlock();
	return copy.rows;
}

cv::Mat webcam_t::frame()
{
	frame_lock_m.lock();
	auto copy=frame_m;
	frame_lock_m.unlock();
	return copy;
}

std::string webcam_t::jpg(const size_t quality)
{
	if(!good())
		return "";

	try
	{
		auto copy=frame();

		if(copy.empty())
			return "";

		std::vector<unsigned char> buffer;
		std::vector<int> params;
		params.push_back(CV_IMWRITE_JPEG_QUALITY);
		params.push_back(quality);
		cv::imencode(".jpg",copy,buffer,params);
		return std::string((char*)buffer.data(),buffer.size());
	}
	catch(...)
	{
		return "";
	}
}

void webcam_t::update_m()
{
	while(good())
	{
		cv::Mat new_frame;
		capture_m>>new_frame;

		if(new_frame.empty())
			break;

		frame_lock_m.lock();
		frame_m=new_frame;
		frame_lock_m.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	close();
}