#ifndef WEBCAM_HPP
#define WEBCAM_HPP

#include <opencv2/opencv.hpp>
#include <string>

class webcam_t
{
	public:
		bool good() const;
		void open(const size_t number);
		void close();
		void update();
		size_t width() const;
		size_t height() const;
		const void* data() const;
		std::string jpg(const size_t quality) const;

	private:
		cv::VideoCapture capture_m;
		cv::Mat frame_m;
};

#endif