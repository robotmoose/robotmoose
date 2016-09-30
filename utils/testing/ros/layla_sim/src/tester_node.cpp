#include "ros/ros.h"
#include "layla_sim/get.h"
#include "layla_sim/set.h"
#include <chrono>
#include <string>
#include <thread>
#include <iostream>

int main(int argc, char **argv) {
	ros::init(argc, argv, "tester_node");

	size_t ITERS=1000;

	if(argc>1)
		ITERS=std::stoi(argv[1]);

	ROS_INFO("Iterations: %lu", ITERS);

	ros::NodeHandle n;
	ros::ServiceClient get_client = n.serviceClient<layla_sim::get>("get");
	layla_sim::get get_srv;

	ros::ServiceClient set_client = n.serviceClient<layla_sim::set>("set");
	layla_sim::set set_srv;

	ROS_INFO("Testing get.");
	std::chrono::duration<double> total_get=std::chrono::duration<double>::zero();
	for(size_t i=0; i<ITERS; ++i) {
		get_srv.request.path = "/";

		auto t0 = std::chrono::high_resolution_clock::now();
		get_client.call(get_srv);
		auto t1 = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		total_get += t1-t0;
	}
	ROS_INFO("Total Time (s): %lf", total_get.count());
	ROS_INFO("Total Time Per Call (ms): %lf", total_get.count()/(double)ITERS*1000);

	ROS_INFO("Testing set.");
	std::chrono::duration<double> total_set=std::chrono::duration<double>::zero();
	for(size_t i=0; i<ITERS; ++i) {
		set_srv.request.path = "/robots/2016/auto/gen/benchmark";
		set_srv.request.value = R"({"poem":"The berries are nice today"})";


		auto t0 = std::chrono::high_resolution_clock::now();
		set_client.call(set_srv);
		auto t1 = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		total_set += t1-t0;
	}
	ROS_INFO("Total Time (s): %lf", total_set.count());
	ROS_INFO("Total Time Per Call (ms): %lf", total_set.count()/(double)ITERS*1000);

	return 0;
}