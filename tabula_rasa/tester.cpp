#include "arduino/robot.h"

robot_t robot;

int main()
{
	while(true)
	{
		robot.loop();
	}

	return 0;
}