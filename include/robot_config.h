#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#include <string>

class robot_config_t
{
	public:
		std::string name;       // superstar robot name
		std::string superstar;  // superstar server
		int baudrate;           // serial comms to Arduino
		std::string motors;     // Arduino firmware device name
		std::string marker;     // computer vision marker file
		std::string sensors;    // All our sensors
		double trim;            // proportional relation between left and right "motor" output
		bool debug;             // more output, more mess, but more data
		int delay_ms;           // milliseconds to wait in control loop (be kind to CPU, network)
		bool sim;               // no arduino, testing purposes

		robot_config_t();

		void from_file(const std::string& filename);
		void from_cli(int argc,char* argv[]);
		void to_file(const std::string& filename) const;
		bool is_valid() const;
};

#endif