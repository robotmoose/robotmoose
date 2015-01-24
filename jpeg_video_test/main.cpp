#include <iostream>
#include <msl/time.hpp>
#include <msl/webserver.hpp>
#include "webcam.hpp"
#include <fstream>

webcam_t cam;

void send_jpg(const mg_connection& connection,const std::string& jpg)
{
	std::string data;
	data+=jpg;
	mg_send_data((mg_connection*)&connection,data.data(),data.size());
}

bool client_func(const mg_connection& connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(event!=MG_REQUEST)
		return false;

	std::string request=connection.uri;

	if(request=="/cam.jpg")
	{
		send_jpg(connection,cam.jpg(30));
		return true;
	}

	std::cout<<request<<std::endl;
	return false;
}

int main()
{
	msl::webserver_t test(client_func,"0.0.0.0:8080","web");
	test.open();

	if(test.good())
		std::cout<<":)"<<std::endl;
	else
		std::cout<<":("<<std::endl;

	while(test.good())
	{
		size_t num=0;
		cam.open(num);

		if(cam.good())
			std::cout<<"Connected with camera "<<num<<"."<<std::endl;

		while(cam.good())
			msl::delay_ms(1);

		std::cout<<"Lost connection with camera "<<num<<"."<<std::endl;
		msl::delay_ms(1);
	}

	test.close();

	return 0;
}