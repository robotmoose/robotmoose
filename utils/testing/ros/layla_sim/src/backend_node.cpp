#include "ros/ros.h"
#include "layla_sim/duplex.h"
#include "layla_sim/test_config.h"

#include <string>

std::string latency_test_data;
std::string bandwidth_test_data;
size_t BYTES_LATENCY_TEST = 128;
size_t BYTES_BANDWIDTH_TEST = 1000000; // 1 MiB

bool test_config(layla_sim::test_config::Request &req, layla_sim::test_config::Response &res) {
	res.bytes_latency_test = BYTES_LATENCY_TEST;
	res.bytes_bandwidth_test = BYTES_BANDWIDTH_TEST;
	return true;
}

bool test_latency(layla_sim::duplex::Request &req, layla_sim::duplex::Response &res) {
	res.response = latency_test_data;
	return true;
}

bool test_bandwidth(layla_sim::duplex::Request &req, layla_sim::duplex::Response &res) {
	res.response = bandwidth_test_data;
	return true;
}

int main(int argc, char **argv) {
	ros::init(argc, argv, "backend_node");

	if(argc > 1)
		BYTES_LATENCY_TEST=std::stoi(argv[1]);
	if(argc > 2)
		BYTES_BANDWIDTH_TEST=std::stoi(argv[2]);

	ROS_INFO("Bytes to send for latency tests: %lu", BYTES_LATENCY_TEST);
	ROS_INFO("Bytes to send for bandwidth tests: %lu", BYTES_BANDWIDTH_TEST);

	latency_test_data = std::string(BYTES_LATENCY_TEST, 'B');
	bandwidth_test_data = std::string(BYTES_BANDWIDTH_TEST, 'B');

	ros::NodeHandle n;

	ros::ServiceServer test_config_service = n.advertiseService("test_config", test_config);
	ros::ServiceServer test_latency_service = n.advertiseService("test_latency", test_latency);
	ros::ServiceServer test_bandwidth_service = n.advertiseService("test_bandwidth", test_bandwidth);
	ROS_INFO("Ready to test latency and bandwidth.");
	ros::spin();

	return 0;
}
