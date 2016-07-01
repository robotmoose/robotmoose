#include <Wire.h>
#include "tabula_control.h"
#include "tabula_config.h"

// Battery Management System
class BMS : public action {
public:
	tabula_sensor<unsigned char> charge; // percent charge, 0-100 percent
	tabula_sensor<unsigned char> state; // current charge state
	enum { bms_addr = 2 }; // Address of BMS on I2C
	enum { numbytes = 2 }; // # of bytes to request from BMS

	BMS() {
		charge=50;
		state=0; // 0: BMS not connected
	}

	virtual void loop() {
		Wire.requestFrom(bms_addr, numbytes);

		for(int ii=0;ii<2;++ii)
		{
			byte x=Wire.read();

			switch(ii)
			{
				case 0:
					charge=x;
					break;
				case 1:
					state=x;
					break;
			}
		}
	}
};