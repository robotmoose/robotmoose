#include "action.h"
#include "motor_controller.h"

uint16_t right_pins[2]={3,5};
uint16_t left_pins[2]={9,6};

class motor_action : public action {
{
public:
	bts_controller_t ctl;
	motor_action() :ctl(left_pins,right_pins) 
	{
		ctl.setup();
	}
	void loop() {
		test.drive(0,0);
	}
};	

void setup()
{
  action_setup();
  action_10ms.add(new motor_action());

  for(int ii=16;ii<=19;++ii)
  {
    pinMode(ii,OUTPUT);
    digitalWrite(ii,HIGH);
  }
}

void loop()
{
  action_loop();
}


