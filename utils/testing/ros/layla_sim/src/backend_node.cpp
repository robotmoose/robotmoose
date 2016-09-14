#include "ros/ros.h"
#include "std_msgs/String.h"

#include <sstream>

void frontendCallback(const std_msgs::String::ConstPtr & msg) {
	ROS_INFO("I heard: [%s]", msg->data.c_str());
}

int main(int argc, char **argv) {
	ros::init(argc, argv, "backend_node");
	ros::NodeHandle n;
	ros::Publisher backend_pub = n.advertise<std_msgs::String>("layla_backend", 1000);
	ros::Subscriber frontend_sub = n.subscribe("layla_frontend",  1000, frontendCallback);
	ros::Rate loop_rate(10);

	int count = 0;
	while(ros::ok()) {
		std_msgs::String msg;
		std::stringstream ss;
		ss << "Hello world from the backend! " << count;

		msg.data = ss.str();
		ROS_INFO("%s", msg.data.c_str());

		backend_pub.publish(msg);

		ros::spinOnce();

		loop_rate.sleep();
		++count;
	}
	return 0;
}