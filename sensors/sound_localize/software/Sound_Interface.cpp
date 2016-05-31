#include <string>
#include <cstdint>
#include <iostream>
#include "serial/serial.h"

int main() {
	// Serial Port Parameters
	std::string port = "/dev/ttyACM1";
	std::uint32_t baudrate = 921600;
	serial::parity_t parity = serial::parity_odd;
	serial::stopbits_t stopbits = serial::stopbits_two;

	// Open the serial port
	serial::Serial sp(
		port,
		baudrate
	);

	// Set additional parameters
	sp.setParity(parity);
	sp.setStopbits(stopbits);

	if(!sp.isOpen()) {
		std::cout << "Error: serial port " + port + " did not open properly!" << std::endl;
		return -1;
	}
	else
		std::cout << "Serial port " + port + " is open."
	std::cout << "Is the serial port open?" << std::endl;
	if(sp.isOpen()) {
		std::cout << "Yes." << std::endl;
		std::cout << sp.getParity() << std::endl;
	}
	else
		std::cout << "No." << std::endl;
	return 0;
}