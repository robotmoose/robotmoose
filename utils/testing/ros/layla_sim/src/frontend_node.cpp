#include "ros/ros.h"
#include "std_msgs/String.h"
#include "layla_sim/duplex.h"
#include "layla_sim/test_config.h"

#include <chrono>
#include <string>
#include <thread>
#include <fstream>

std::string latency_test_data;
std::string bandwidth_test_data;

int main(int argc, char **argv) {
	ros::init(argc, argv, "frontend_node");

	std::ofstream ostr_128("128");
	std::ofstream ostr_1mb("1mb");

	size_t ITERS=1000;

	if(argc>1)
		ITERS=std::stoi(argv[1]);

	ROS_INFO("Iterations: %lu", ITERS);

	ros::NodeHandle n;

	// Get the amount of data to send for each test from the backend node
	ros::ServiceClient test_config_client = n.serviceClient<layla_sim::test_config>("test_config");
	layla_sim::test_config test_config_srv;
	test_config_client.call(test_config_srv);

	latency_test_data  = std::string(test_config_srv.response.bytes_latency_test,'A');
	bandwidth_test_data  = std::string(test_config_srv.response.bytes_bandwidth_test,'A');

	ROS_INFO("Bytes to send for latency tests: %lu", test_config_srv.response.bytes_latency_test);
	ROS_INFO("Bytes to send for bandwidth tests: %lu\n", test_config_srv.response.bytes_bandwidth_test);

	//

	ros::ServiceClient test_latency_client = n.serviceClient<layla_sim::duplex>("test_latency");
	ros::ServiceClient test_bandwidth_client = n.serviceClient<layla_sim::duplex>("test_bandwidth");
	layla_sim::duplex test_srv;

	// Perform the actual tests
	ROS_INFO("Testing Network Latency.");
	test_srv.request.data = latency_test_data;
	std::chrono::duration<double> total_test_latency=std::chrono::duration<double>::zero();
	for(size_t i=0; i<ITERS; ++i) {
		auto t0 = std::chrono::high_resolution_clock::now();
		test_latency_client.call(test_srv);
		auto t1 = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		total_test_latency += t1-t0;
		ostr_128<<(t1-t0).count()<<std::endl;
	}
	ROS_INFO("Total Time (s): %lf", total_test_latency.count());
	ROS_INFO("Average Time Per Call (ms): %lf\n", total_test_latency.count()/(double)ITERS*1000);


	ROS_INFO("Testing Network Bandwidth.");
	test_srv.request.data = bandwidth_test_data;
	std::chrono::duration<double> total_test_bandwidth=std::chrono::duration<double>::zero();
	for(size_t i=0; i<ITERS; ++i) {
		auto t0 = std::chrono::high_resolution_clock::now();
		test_bandwidth_client.call(test_srv);
		auto t1 = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		total_test_bandwidth += t1-t0;
		ostr_1mb<<(t1-t0).count()<<std::endl;
	}
	ROS_INFO("Total Time (s): %lf", total_test_bandwidth.count());
	ROS_INFO("Total Time Per Call (ms): %lf\n", total_test_bandwidth.count()/(double)ITERS*1000);
	ostr_128.close();
	ostr_1mb.close();

	return 0;
}
