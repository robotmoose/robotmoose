/*
  Send and receive tabula_control command and sensor values,
  using A-packet serial comms.
 
  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-19 (Public Domain) 
*/
#include "action.h"
#include "tabula_config.h"
#include "tabula_control.h"
#include "serial_packet.h"


// Static value storage for sensor and command data:
tabula_control_storage tabula_sensor_storage;
tabula_control_storage tabula_command_storage;


/** This class manages communication via an A_packet_formatter,
 including timeouts. */
class serial_channel {
public:
	HardwareSerial &serial;
	A_packet_formatter<HardwareSerial> pkt; // packet formatter
	A_packet p; // last received packet
	bool is_connected; // 1 if we're recently connected; 0 if no response
	uint32_t last_read; // millis() the last time we got data back
	uint32_t next_send; // millis() the next time we should send off data

	serial_channel(HardwareSerial &serial_) 
		:serial(serial_), pkt(serial)
	{
		is_connected=0;
		last_read=0;
		next_send=0;
		p.valid=0;
	}

	// Poll on the serial device.  If a full packet is available, return true.
	bool read_packet(void) {
		if (serial.available()) {
			while (-1==pkt.read_packet(p)) { }
			if (p.valid) {
				last_read=action_time_ms;
				next_send=action_time_ms+500;
				is_connected=true; // got valid packet
				return true;
			}
		}
		if (action_time_ms>next_send) { // read timeout
			next_send=action_time_ms+500;
			pkt.reset();
			pkt.write_packet(0,4,"ping"); // send heartbeat ping packet
			is_connected=false;
		}
		return false;
	}
};


/**
 The serial controller is what polls on the serial port
 to actually send and receive packets.
*/
class serial_controller : public action {
public:
	serial_channel channel;
	serial_controller(HardwareSerial &serial) :channel(serial) {}
	
	virtual void loop() {
		if (channel.read_packet()) 
		{ // Respond to packet request
			if (channel.p.command==0xC) {
				handle_command_packet(channel.p);
			}
			else { // unknown request type
				channel.pkt.write_packet(0xE,1,"?");
			}
		}
	}
	
	// The PC has command data for us.  Send back sensor data.
	void handle_command_packet(A_packet &p) {
		if (p.length!=tabula_command_storage.count) {
			channel.pkt.write_packet(0xE,4,"Clen");
			return;
		}
		memcpy(tabula_command_storage.array,p.data,p.length);
		channel.pkt.write_packet(0xC,
			tabula_sensor_storage.count,
			tabula_sensor_storage.array);
	}
};

REGISTER_TABULA_DEVICE(serial_controller,
	actions_10ms.add(new serial_controller(Serial));
)


/**
 Scalable command and sense system for robot control:

command: PC sending control commands for motor controllers, 
  enable bits, etc.

sensor: Arduino sending temperature, ultrasonic, bumpers, etc.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-18 (Public Domain)
*/
#include "tabula_control.h"
#include "tabula_config.h"


// Example devices moved to devices.cpp
