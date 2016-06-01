#include <string>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm> // std::for_each
#include "serial/serial.h"

int main() {
	// List available serial ports
	std::vector<serial::PortInfo> portInfo = serial::list_ports();
	std::string port = "";

	for(auto it = portInfo.begin(); it != portInfo.end(); ++it) {
		if((it -> description).find("Arduino") || (it -> description).find("arduino")) {
			port = it -> port;
			break;
		}
	}

	// Serial Port Parameters
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
	else std::cout << "Serial port " + port + " is open." << std::endl;

	sp.close();
	return 0;
}