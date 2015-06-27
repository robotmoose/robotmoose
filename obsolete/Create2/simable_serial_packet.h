
#ifndef __SIMABLE_SERIAL_APACKET__H
#define __SIMABLE_SERIAL_APACKET__H

#include "serial_packet.h"

template <class serial_port>
class simable_A_packet_formatter {
public:
	enum { max_command = 15 };
	enum { max_short_length = 15 };

	/**
	Create an A-packet send/receive object for this serial device.
	The device must be ready to send/receive bytes.
	*/
	simable_A_packet_formatter(serial_port &serial_) :myPacket(serial_), sim(false)
	{
	}

	simable_A_packet_formatter(serial_port &serial_, bool areWeSim) :myPacket(serial_)
	{
		sim = areWeSim;
	}
	/// Reset packet receive state to start-of-packet
	void reset() {
		myPacket.reset();
	}


	/* Packet send */
	virtual void write_packet(int16_t command, int16_t length, const void *data)
	{
		if (sim)
		{

		}
		else
			myPacket.write_packet(command, length, data);
	}

	/* Packet receive */

	/**
	This function processes serial bytes and reassembles packets.
	Returns 0 if no data is available to read right now.
	Returns -1 if data was readable, but no packet is ready yet (call again).
	Fills out the packet and returns +1 if we received a correctly formatted packet.

	Idiomatic call code:
	A_packet p;
	while (-1==apak.read_packet(p)) {} // keep reading
	if (p.valid) { // handle packet
	}
	*/
	virtual int16_t read_packet(A_packet &p)
	{
		if (sim)
		{
			return 0;
		}
		else
		{
			return myPacket.read_packet(p);
		}
	}
private:
	A_packet_formatter<serial_port> myPacket;
	bool sim;
};



#endif

