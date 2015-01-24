#include <fstream>

#include <iostream>
#include "webcam.hpp"

int main()
{
	while(true)
	{
		size_t num=0;
		webcam_t cam;
		cam.open(num);

		if(cam.good())
			std::cout<<"Connected with camera "<<num<<"."<<std::endl;

		while(cam.good())
		{
			cam.update();

			std::ofstream ostr("test.jpg");
			ostr<<cam.jpg(40);
			ostr.close();
		}

		std::cout<<"Lost connection with camera "<<num<<"."<<std::endl;


	}

	return 0;
}