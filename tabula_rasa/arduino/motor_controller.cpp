#include "motor_controller.h"
#include "tabula_config.h"

void motor_controller_t::loop()
{
	drive(left.get(), right.get());
}

bts_controller_t::bts_controller_t(uint16_t left_pwms[2],uint16_t right_pwms[2])
{
	for(int16_t ii=0;ii<2;++ii)
	{
		left_pwms_m[ii]=left_pwms[ii];
		right_pwms_m[ii]=right_pwms[ii];
	}
	for(int16_t ii=0;ii<2;++ii)
	{
		pinMode(left_pwms_m[ii],OUTPUT);
		pinMode(right_pwms_m[ii],OUTPUT);
		digitalWrite(left_pwms_m[ii],LOW);
		digitalWrite(right_pwms_m[ii],LOW);
	}
}

void bts_controller_t::drive(const int16_t left,const int16_t right)
{
	for(int16_t ii=0;ii<2;++ii)
	{
		digitalWrite(left_pwms_m[ii],LOW);
		digitalWrite(right_pwms_m[ii],LOW);
	}

	analogWrite(left_pwms_m[left>=0],abs(left));
	analogWrite(right_pwms_m[right>=0],abs(right));
}

sabertooth_controller_t::sabertooth_controller_t(Stream& serial):
	serial_m(&serial)
{}

void sabertooth_controller_t::drive(const int16_t left,const int16_t right)
{
	send_motor_m(128,6,left);
	send_motor_m(128,7,right);
}

sabertooth_v1_controller_t::sabertooth_v1_controller_t(Stream& serial):
	sabertooth_controller_t(serial)
{}

void sabertooth_v1_controller_t::send_motor_m(const uint8_t address,const uint8_t motor,const int16_t value)
{
	uint8_t value_raw=64+(value>>2);

	if(motor==7)
		value_raw+=128;

	serial_m->write(&value_raw,1);
}

sabertooth_v2_controller_t::sabertooth_v2_controller_t(Stream& serial):
	sabertooth_controller_t(serial)
{}

//untested
void sabertooth_v2_controller_t::send_motor_m(const uint8_t address,const uint8_t motor,const int16_t value)
{
	uint8_t value_raw=64+(value>>2);

	uint8_t data[4]=
	{
		address,
		motor,
		value_raw,
		(uint8_t)((address+motor+value_raw)&127)
	};

	serial_m->write(data,4);
}

/// Register our devices with tabula_setup
REGISTER_TABULA_DEVICE(bts, "PPPP",
	uint16_t left[2]; uint16_t right[2]; 
	left[0]=src.read_pin();	left[1]=src.read_pin();
	right[0]=src.read_pin(); right[1]=src.read_pin();
	actions_10ms.add(new bts_controller_t(left,right));
)

REGISTER_TABULA_DEVICE(sabertooth1, "S",
	Stream *saber=src.read_serial(9600);
	actions_10ms.add(new sabertooth_v1_controller_t(*saber));
)

REGISTER_TABULA_DEVICE(sabertooth2, "S",
	Stream *saber=src.read_serial(9600);
	delay(2000); // datasheet: wait 2 seconds after startup
	saber->write(0xAA); // send the "bauding character"
	delay(10); // paranoia
	actions_10ms.add(new sabertooth_v2_controller_t(*saber));
)

/** iRobot Create 2 / Roomba 500/600 series */
class create2_controller_t : public motor_controller_t
{
public:
	create2_controller_t(roomba_t& roomba,int BRC_pin_);
	virtual void loop();
	void drive(const int16_t left,const int16_t right);

private:
	roomba_t* roomba_m;
	int BRC_pin;
	tabula_sensor<roomba_t::sensor_t> sensors_m;
	bool floorPresent; 
};

create2_controller_t::create2_controller_t(roomba_t& roomba,int BRC_pin_)
	:roomba_m(&roomba), BRC_pin(BRC_pin_), floorPresent(true)
{}

void create2_controller_t::loop()
{
	roomba_m->update();
	sensors_m=roomba_m->get_sensors();
	bool go_passive=false;
	static bool was_connected=false; // true if we've ever seen the PC connected
	static bool was_disconnected=false; // true if we manually disconnected ourselves
	if (pc_connected) was_connected=true;

	// Button press -> disconnect
	if (roomba_m->get_sensors().buttons&roomba_t::BUTTON_CLEAN
	  ||roomba_m->get_sensors().buttons&roomba_t::BUTTON_DOCK) 
		go_passive=true;
	
	// PC disconnect
	if (!pc_connected && was_connected)
	{
		go_passive=true;
		was_disconnected=true;
	}
	
	if (go_passive)
	{ // power button is pressed--enter Passive mode, so it will charge
		roomba_m->set_mode(roomba_t::PASSIVE);
	}
	
	if (pc_connected && was_disconnected) 
	{ // reconnect
		was_disconnected=false;
		roomba_m->setup(BRC_pin);
	}
	
	// Check floor sensors
        floorPresent = true;
	for(int i=0; i<4; ++i)
	{
		if(roomba_m->get_sensors().floor[i] < 3)
		{
			floorPresent = false;
		}
	}
	
	motor_controller_t::loop();
}

void create2_controller_t::drive(const int16_t left,const int16_t right)
{
	int l = left;
	int r = right;
	
	// Don't allow forward motion if floor not present
	if (!floorPresent &&( left > 0 || right >0))
		l=r=0;

	roomba_m->drive(l<<1,r<<1); // multiply commands by 2, to go from +-256 to +-512
}


REGISTER_TABULA_DEVICE(create2, "S",
	Stream *roomba_serial=src.read_serial(115200);
	int BRC_pin=2; // assume baud rate control (BRC) is jumpered to this pin
	roomba_t* roomba=new roomba_t (*roomba_serial);
	Serial.println("Resetting roomba (push CLEAN if this step hangs!)");
	roomba->setup(BRC_pin);
	Serial.println("Roomba reset");

	actions_10ms.add(new create2_controller_t(*roomba,BRC_pin));
)


