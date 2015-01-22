#include "arduino.hpp"

#include <chrono>
#include <iostream>
#include <thread>

arduino_t::arduino_t(const std::string& serial,const size_t baud):
	serial_m(serial),baud_m(baud),arduino_m(serial_m,baud_m)
{}

arduino_t::~arduino_t()
{
	stop();
}

bool arduino_t::good() const
{
	return arduino_m.good();
}

void arduino_t::start(const bool detach)
{
	stop();
	arduino_m.open(serial_m,baud_m);
	std::thread arduino_thread(&arduino_t::arduino_thread_func_m,this);

	if(detach)
		arduino_thread.detach();
	else
		arduino_thread.join();
}

void arduino_t::stop()
{
	arduino_m.close();
}

void arduino_t::write(const std::string& buffer)
{
	write_lock_m.lock();
	arduino_m.write(buffer);
	write_lock_m.unlock();
}

std::string arduino_t::get_serial() const
{
	return arduino_m.name();
}

void arduino_t::set_serial(const std::string& serial,const bool restart)
{
	serial_m=serial;

	if(restart)
	{
		stop();
		start();
	}
}

size_t arduino_t::get_baud() const
{
	return arduino_m.baud();
}

void arduino_t::set_baud(const size_t baud,const bool restart)
{
	baud_m=baud;

	if(restart)
	{
		stop();
		start();
	}
}

void arduino_t::arduino_thread_func_m()
{
	while(true)
	{
		arduino_m.open();

		if(arduino_m.good())
		{
			std::cout<<"Arduino found on "<<arduino_m.name()<<"@"<<arduino_m.baud()<<"."<<std::endl;

			while(arduino_m.good())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		arduino_m.close();
		std::cout<<"Arduino not found on "<<arduino_m.name()<<"@"<<arduino_m.baud()<<"."<<std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}