#include "ros/ros.h"
#include "std_msgs/String.h"

#include <sstream>

void backendCallback(const std_msgs::String::ConstPtr & msg) {
	ROS_INFO("I heard: [%s]", msg->data.c_str());
}

int main(int argc, char **argv) {
	ros::init(argc, argv, "frontend_node");
	ros::NodeHandle n;
	ros::Publisher frontend_pub = n.advertise<std_msgs::String>("layla_frontend", 1000);
	ros::Subscriber backend_sub = n.subscribe("layla_backend", 1000, backendCallback);
	ros::Rate loop_rate(10);

	int count = 0;
	while(ros::ok()) {
		std_msgs::String msg;
		std::stringstream ss;
		ss << "Hello world from the frontend! " << count;

		msg.data = ss.str();
		ROS_INFO("%s", msg.data.c_str());

		frontend_pub.publish(msg);

		ros::spinOnce();

		loop_rate.sleep();
		++count;
	}
	return 0;
}