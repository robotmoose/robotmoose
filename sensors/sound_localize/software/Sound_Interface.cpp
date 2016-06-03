// Sound_Interface.cpp
// Processes data sent from microphone array via Arduino
// Ryker Dial
// UAF ITEST

// Date Created: 5/31/2016
// Last Modified: 6/3/2016

#include <string>
#include <cstdint>
#include <iostream> // for std::cout, std::endl
#include <vector>
#include <deque>
#include <cstdio> // for std::printf & std::sprintf
#include <utility> // for std::pair
#include "serial/serial.h"
#include <fftw3.h>

#include <fstream>

int main() {

	// ********** Serial Port Setup Begin ********** //
	// Serial Port Parameters
	std::string port = "";
	std::uint32_t baudrate = 921600;
	serial::Timeout timeout = serial::Timeout::simpleTimeout(250);
	serial::bytesize_t bytesize = serial::eightbits;
	serial::parity_t parity = serial::parity_odd;
	serial::stopbits_t stopbits = serial::stopbits_two;
	serial::flowcontrol_t flowcontrol = serial::flowcontrol_none;

	//Get Arduino port name automatically
	std::vector<serial::PortInfo> portInfo = serial::list_ports();
	for(auto it = portInfo.begin(); it != portInfo.end(); ++it) {
		std::cout << it -> description << std::endl;
		std::cout << it -> port << std::endl;
		if((it -> description).find("Arduino") 
			|| (it -> description).find("arduino")
			|| (it -> description).find("USB2.0-Serial")) { // Generic Nano
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
	// ********** Serial Port Setup End ********** // 


	// ********** Data Parsing and Processing Setup Begin ********** // 
	// Variables and containers needed for parsing the serial data
	std::size_t num_streams = 5; // Number of data streams = number of microphones

	std::size_t data_length = num_streams*(2)+4; // 2 bytes per microphone + 4 bytes for time
	std::uint8_t buffer[100];
	std::uint32_t time_micros;
	std::vector<std::uint16_t> microphone_data(num_streams);

	std::uint16_t fs = 7800; // ADC Sampling Frequency
	// Number of samples to use for FFT. FFT resolution is fs/N.
	std::size_t num_samples_fft = 2048;
	// Counts up to num_samples_fft, then triggers FFT. Is reset to N/2 so FFTs overlap.
	std::size_t fft_counter = 0; 
	
	// Create a vector of data streams. Each data stream is a deque containing the received
	//     microphone readings.
	std::vector<std::deque<std::uint16_t>> data_streams;
	for(int i=0; i<num_streams; ++i) {
		data_streams.push_back(std::deque<std::uint16_t>());
	}

	// Open up files for data logging
	std::ofstream output_time("output_time.csv", std::ios::out | std::ios::trunc);
	std::ofstream output_freq("output_freq.csv", std::ios::out | std::ios::trunc);
	if(!(output_time.is_open()) || !(output_freq.is_open())) {
		std::cout << "Error: Unable to open log files!" << std::endl;
		return -1;
	}
	char buff[50];

	// Set up mechanisms for FFT
	std::vector<std::pair<double *, double *>> fft_data;

	for(int i=0; i<num_streams; ++i) {
		fft_data.push_back(std::make_pair(
			(double*) fftw_malloc(sizeof(double) * num_samples_fft),
			(double*) fftw_malloc(sizeof(double) * num_samples_fft)
		));
	}
	//in = (double*) fftw_malloc(sizeof(double) * num_samples_fft);
	//out = (double*) fftw_malloc(sizeof(double) * num_samples_fft);
	// Create optimized plan for executing FFT
	fftw_plan p = fftw_plan_r2r_1d(
		num_samples_fft, 
		fft_data[0].first, 
		fft_data[0].second, 
		FFTW_R2HC, 
		FFTW_MEASURE
	);

	// ********** Data Parsing and Processing Setup End ********** // 


	// ********** Main Program Loop ********** //
	while(true) {
		// Syncronize by reading CR (10) followed by LF (13)
		sp.read(buffer,1);
		if(buffer[0] == 10) {
			sp.read(buffer,1);
			if(buffer[0] == 13) { // Synchronized
				sp.read(buffer, data_length);
				++fft_counter;

				// Reconstruct time data
				time_micros = 0;
				for(int i=0; i<4; ++i) {
					time_micros += buffer[i] << 8*(3-i);
				}
				//std::printf("Time data: %u\n", time_micros);
				for(int i=0; i<num_streams; ++i) {
					microphone_data[i] = 0;
					// Reconstruct microphone data
					for(int j=0; j<2; ++j) {
						microphone_data[i] += buffer[4+j+i*2] << 8*(1-j);
					}
					// Store data for use with FFT
					data_streams[i].push_back(microphone_data[i]);
					if(data_streams[i].size() > num_samples_fft) data_streams[i].pop_front();
					
					std::sprintf(buff, "%u,", microphone_data[i]);
					output_time << buff;
					//std::printf("Microphone data %d: %u\n", i, microphone_data[i]);
				}
				//std::printf("Counter: %lu\n", fft_counter);
				if(fft_counter >= num_samples_fft) {
					// Execute the Fourier transforms
					for(int i=0; i<num_streams; ++i) {
						for(int j=0; j<num_samples_fft; ++j) {
							// Copy microphone data to in. This will convert int to double.
							fft_data[i].first[j] = (double) data_streams[i][j];
						}
						//clearfftw_execute(p); // perform the fft
						fftw_execute_r2r(p, fft_data[i].first, fft_data[i].second);
						std::printf("FFT #%d performed.\n", i);
					}
					// Log the FFT outputs.
					for(int i=0; i<(num_samples_fft+1)/2; ++i) {
						for(int j=0; j<num_streams; ++j) {
							std::sprintf(buff, "%f,", fft_data[j].second[i]);
							output_freq << buff;			
						}
						output_freq << "\n";
					}
					fft_counter = num_samples_fft/2;
				}
				output_time << "\n";
			}
		}
	}
	// Clean Up
	sp.close();
	fftw_destroy_plan(p);
	for(int i=0; i<num_streams; ++i) {
		fftw_free(fft_data[i].first);
		fftw_free(fft_data[i].second);
	}
	return 0;
}