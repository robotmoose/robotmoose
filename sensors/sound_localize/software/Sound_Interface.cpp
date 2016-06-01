// Sound_Interface.cpp
// Processes data sent from microphone array via Arduino
// Ryker Dial
// UAF ITEST

// Date Created: 5/31/2016
// Last Modified: 6/1/2016

#include <string>
#include <cstdint>
#include <iostream> // for std::cout, std::endl
#include <vector>
#include <cstdio> // for std::printf
#include "serial/serial.h"

int main() {
	// Serial Port Parameters
	std::string port = "";
	std::uint32_t baudrate = 921600;
	serial::Timeout timeout = serial::Timeout::simpleTimeout(250);
	serial::bytesize_t bytesize = serial::eightbits;
	serial::parity_t parity = serial::parity_odd;
	serial::stopbits_t stopbits = serial::stopbits_two;
	serial::flowcontrol_t flowcontrol = serial::flowcontrol_none;

	// Get Arduino port name automatically
	std::vector<serial::PortInfo> portInfo = serial::list_ports();
	for(auto it = portInfo.begin(); it != portInfo.end(); ++it) {
		if((it -> description).find("Arduino") || (it -> description).find("arduino")) {
			port = it -> port;
			break;
		}
	}

	// Open the serial port
	serial::Serial sp(
		port,
		baudrate,
		timeout,
		bytesize,
		parity,
		stopbits,
		flowcontrol
	);

	// Check if the port was opened successfully
	if(!sp.isOpen()) {
		std::cout << "Error: serial port " + port + " did not open properly!" << std::endl;
		return -1;
	}
	else std::cout << "Serial port " + port + " is open." << std::endl;

	// Variables and containers needed for parsing the serial data
	std::size_t num_microphones = 5;
	std::size_t data_length = num_microphones*(4+2);
	std::uint8_t buffer[100];
	std::vector<std::uint32_t> time_micros(num_microphones);
	std::vector<std::uint16_t> microphone_data(num_microphones);

	while(true) {
		sp.read(buffer,1);
		if(buffer[0] == 10) {
			sp.read(buffer,1);
			if(buffer[0] == 13) { // Synchronized
				std::cout << "Synchronized" << std::endl;
				sp.read(buffer, data_length);

				for(int i=0; i<num_microphones; ++i) {
					time_micros[i] = 0;
					microphone_data[i] = 0;
					// Reconstruct time data
					for(int j=0; j<4; ++j) {
						time_micros[i] += buffer[j+i*(4+2)] << 8*(3-j);
					}
					// Reconstruct microphone data
					for(int j=0; j<2; ++j) {
						microphone_data[i] = buffer[j+4+i*(4+2)] << 8*(1-j);
					}
					std::printf("Time data %d: %u\n", i, time_micros[i]);
					std::printf("Microphone data %d: %u\n", i, microphone_data[i]);
				}
			}
		}
	}
	sp.close();
	return 0;
}