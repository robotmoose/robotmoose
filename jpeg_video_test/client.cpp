#include <iostream>
#include <msl/time.hpp>
#include <msl/webserver.hpp>
#include "webcam.hpp"
#include <mutex>
#include <stdexcept>

void send_jpg(const mg_connection& connection,const std::string& jpg);

bool client_func(const mg_connection& connection,enum mg_event event);

msl::webserver_t server(client_func,"0.0.0.0:8081","web");
webcam_t cam;
std::string jpg="";
std::mutex jpg_lock;

int main()
{
	server.open();

	if(server.good())
		std::cout<<":)"<<std::endl;
	else
		std::cout<<":("<<std::endl;

	while(server.good())
	{
		size_t num=0;
		cam.open(num);

		if(cam.good())
			std::cout<<"Connected with camera "<<num<<"."<<std::endl;

		while(cam.good())
		{
			msl::delay_ms(1);
			auto copy=cam.jpg(15);
			jpg_lock.lock();
			jpg=copy;
			jpg_lock.unlock();
		}

		std::cout<<"Lost connection with camera "<<num<<"."<<std::endl;
		msl::delay_ms(1);
	}

	server.close();

	return 0;
}

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

	std::cout<<connection.uri<<std::endl;

	if(std::string(connection.uri)=="/cam.jpg")
	{
		jpg_lock.lock();
		auto copy=jpg;
		jpg_lock.unlock();
		send_jpg(connection,copy);
		return true;
	}

	return false;
}
