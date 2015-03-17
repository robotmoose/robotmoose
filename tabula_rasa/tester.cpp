#include "arduino/robot.h"

bts_controller_t bts(3,4);
robot_t robot(&bts);

int main()
{
	while(true)
	{
		robot.loop();
	}

	return 0;
}