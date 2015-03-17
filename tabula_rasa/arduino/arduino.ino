#include "robot.h"

bts_controller_t bts(3,4);
robot_t robot(&bts);

void setup()
{
}

void loop()
{
  robot.loop();
}
