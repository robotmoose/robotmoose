#include "action.h"
#include "motor_controller.h"
#include <SoftwareSerial.h>

uint16_t right_pins[2]={3,5};
uint16_t left_pins[2]={9,6};

class motor_action : public action {
public:

#ifdef __USE_BTS__
	bts_controller_t ctl;
	motor_action() :ctl(left_pins,right_pins) 
	{
#else // sabertooth
# if 0 // software serial
	SoftwareSerial saberserial;
	sabertooth_v1_controller_t ctl;
	motor_action() :saberserial(9,8), ctl(saberserial)
	{
# else
#  define saberserial Serial1
	sabertooth_v1_controller_t ctl;
	motor_action() :ctl(saberserial)
	{
# endif
		saberserial.begin(9600);
		delay(2000);
		saberserial.write(0xAA); // "bauding character"
		delay(10); // paranoia
#endif
	
		ctl.setup();
	}
	void loop() {
		ctl.drive(0,0);
	}
};

class still_alive : public action {
public:
   void loop() { Serial.println("Still alive!\n"); }
};

void setup()
{
  Serial.begin(57600);
  
  action_setup();
  actions_10ms.add(new motor_action());
  actions_1s.add(new still_alive());

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


