#include <string>
#include <cstdint>
#include <iostream>
#include "serial/serial.h"

int main() {

	std::string port = "/dev/ttyACM1";
	std::uint32_t baudrate = 921600;

/*	serial::Serial sp(
		const std::string &port = "/dev/ttyACM1",
		uint32_t baudrate = 921600,
		serial::parity_t parity = serial::parity_odd,
		serial::stopbits_t stopbits = serial::stopbits_two

	);*/
	serial::Serial sp(
		port,
		baudrate
	);
	std::cout << "Is the serial port open?" << std::endl;
	if(sp.isOpen())
		std::cout << "Yes." << std::endl;
	else
		std::cout << "No." << std::endl;
	return 0;
}