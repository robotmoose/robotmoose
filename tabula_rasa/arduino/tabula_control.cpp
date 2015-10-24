/*
  Send and receive tabula_control command and sensor values,
  using A-packet serial comms.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-19 (Public Domain)
*/
#include "action.h"
#include "tabula_config.h"
#include "tabula_control.h"
#include "serial_packet.h"

// Global to expose visibility
bool pc_connected=false;

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
	milli_t last_read; // millis() the last time we got data back
	milli_t last_send; // millis() the last time we sent off ping data

	serial_channel(HardwareSerial &serial_)
		:serial(serial_), pkt(serial)
	{
		pc_connected=false;
		last_read=0;
		last_send=0;
		p.valid=0;
	}

	// Poll on the serial device.  If a full packet is available, return true.
	bool read_packet(void) {
		if (serial.available()) {
			while (-1==pkt.read_packet(p)) { }
			if (p.valid) {
				last_read=last_send=action_time_ms;
				pc_connected=true; // got valid packet
				return true;
			}
		}
		if (action_time_ms-last_send>1000) { // comms timeout
			last_send=action_time_ms;
			
			// Zero our incoming command buffer (stops motors)
			tabula_command_storage.zero();
			
			// Flush serial buffers and try to re-synchronize
			pkt.reset();
			pkt.write_packet(0,4,"ping"); // send heartbeat ping packet
			pc_connected=false;
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
			else if (channel.p.command==0xB) {
				void(*reset)()=0;
				reset();
			}
			else { // unknown request type--send debug
				channel.pkt.write_packet(0xD,12,"bad req type");
			}
		}
	}

	// The PC has command data for us.  Send back sensor data.
	void handle_command_packet(A_packet &p) {
		if (p.length!=tabula_command_storage.count) {
			channel.pkt.write_packet(0xD,11,"bad cmd len");
			return;
		}
		memcpy(tabula_command_storage.array,p.data,p.length);
		channel.pkt.write_packet(0xC,
			tabula_sensor_storage.count,
			tabula_sensor_storage.array);
	}
};

REGISTER_TABULA_DEVICE(serial_controller,"",
	HardwareSerial &channel=Serial;
	channel.println();
	channel.println(-1); // switching to binary mode
	actions_10ms.add(new serial_controller(channel));
)

