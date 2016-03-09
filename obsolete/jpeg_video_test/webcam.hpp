#ifndef WEBCAM_C11_HPP
#define WEBCAM_C11_HPP

//Required Libraries:
//	-lopencv_core -lopencv_highgui -lpthread

#include <opencv2/opencv.hpp>
#include <mutex>
#include <string>

class webcam_t
{
	public:
		webcam_t()=default;
		webcam_t(const webcam_t& copy)=delete;
		~webcam_t();
		webcam_t& operator=(const webcam_t& copy)=delete;
		bool good() const;
		void open(const size_t number);
		void close();
		size_t width();
		size_t height();
		cv::Mat frame();
		std::string jpg(const size_t quality);

	private:
		void update_m();
		cv::VideoCapture capture_m;
		cv::Mat frame_m;
		std::mutex frame_lock_m;
};

#endif