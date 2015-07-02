/**
Goals for the A-packet library:
	- Implicit flow control via synchronization
	- Easy to recover from mis-synchronization
	- Detects wire errors using a checksum
	- Low CPU and comm overhead
	- Compatible with Arduino or PC using same codebase

Simple request-response structure:
	- PC sends request packet: actuator commands, status request, etc.
	- Arduino sends a response packet: ack, sensor values, etc.

Packet structure:
	- 4-bit sync code (0xA)
	- 4-bit packet payload length (if 0xF, first byte of payload is real length)
		- optional 8-bit payload length
	... payload bytes ...
	- 4-bit command code
	- 4-bit checksum: equal to low 4 bits of (real_length+command_code+sumpay+(sumpay>>4)),
		where sumpay is the 8-bit arithmetic sum of the bytes in the payload.

Current robot command codes:
	0xE: Error, sent from robot to PC (for firmware debugging)
	0xD: Depth data request/response, for Neato laser distance sensor
		Request data is empty
		Response data is an entire NeatoLDSbatch struct
	0xA: NOT RECOMMENDED (command code matches the packet start sync code)
	0x5: Sensor request (5 looks like backwards S)
		Request data is empty
		Response data is a list of sensor values
	0x3: Motor command (3 looks like sideways M)
		Request data is list of motor/actuator power bytes
		Response data is empty (or a list of wheel encoder counts?)
	0x0: OK heartbeat, sent from robot to PC spontaneously, or on a 0x0 request

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-19 version (Public Domain)
*/
#ifndef __CYBERALASKA_SERIAL_APACKET__H
#define __CYBERALASKA_SERIAL_APACKET__H

#include <stdlib.h> /* for realloc and free */
#include <string.h> /* for memcpy */

/** Data about one packet */
class A_packet {
public:
	unsigned char valid; ///< 1 if packet is correctly formatted
	int command; ///< Command number.  Must be less than 16.
	int length; ///< Command length.  Must be less than 250.
	const unsigned char *data; ///< Command's associated data.

	A_packet() {
		valid=0;
		command=0;
		length=0;
		data=0;
	}

	/// Copy the packet data into this object.
	///   If the data size doesn't match, return false.
	template <class T>
	bool get(T &target) const {
		if (sizeof(T)!=length || valid==0) return false;
		memcpy(&target,data,length);
		return true;
	}
};

/** Sends and receives A-packets via a serial byte stream. */
template <class serial_port>
class A_packet_formatter {
public:
	enum {max_command=15};
	enum {max_short_length=15};
	serial_port &serial;

	/**
	  Create an A-packet send/receive object for this serial device.
	  The device must be ready to send/receive bytes.
	*/
	A_packet_formatter(serial_port &serial_)
		:serial(serial_)
	{
		reset();
		read_data_long=read_data=NULL;
	}
	/// Reset packet receive state to start-of-packet
	void reset() {
		read_state=0; read_length=0; read_index=0; read_sumpay=0;
	}
	~A_packet_formatter() {
		if (read_data_long) { free(read_data_long); read_data_long=NULL; }
	}

/* Packet send */
	void write_packet(int command,int length,const void *data) {
		const unsigned char *cdata=(const unsigned char *)data;
		unsigned char start=0xA0;
		int sumpay=0;
		for (int i=0;i<length;i++) sumpay+=cdata[i];
		int checksum=0xf&(length+command+sumpay+(sumpay>>4));
		unsigned char end=(command<<4)+(checksum);

		if (length<max_short_length)
		{ // send short packets in one big buffer
			start+=length;
			write_data[0]=start;
			for (int i=0;i<length;i++) write_data[1+i]=cdata[i];
			write_data[1+length]=end;
			serial.write(&write_data[0],2+length);
		} else { // send longer packets in-place
			start+=max_short_length;
			write_data[0]=start;
			write_data[1]=length; // real length byte at start
			serial.write(&write_data[0],2);
			serial.write(cdata,length);
			serial.write(&end,1);
		}
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
	int read_packet(A_packet &p) {
		if (!serial.available()) return 0; // no data to read
		int c=serial.read(); // else read next byte
		if (c==-1) return 0; // (hmmm, why did available return true then?)
		p.valid=0;

		enum {
			STATE_START=0,
			STATE_LENGTH,
			STATE_PAYLOAD,
			STATE_END
		};

		switch (read_state) {
		case 0: /* start byte */
			if ((c&0xf0) == 0xa0) { // valid start code
				read_index=0;
				read_sumpay=0;
				read_length=c&0x0f;
				if (read_length>=max_short_length)
				{
					read_state=STATE_LENGTH; // need real length byte
				}
				else if (read_length>0) {
					read_state=STATE_PAYLOAD; // short payload data
					read_data=&read_data_short[0];
				} else { // length==0, no payload
					read_state=STATE_END;
				}
			}
			break;
		case STATE_LENGTH: /* (optional) length byte */
			read_length=c;
			read_data_long=(unsigned char *)realloc(read_data_long,read_length);
			read_data=read_data_long;
			read_state=STATE_PAYLOAD;
			if(read_length==0)
				read_state=STATE_END;
			break;
		case STATE_PAYLOAD: /* payload data */
			read_sumpay+=c;
			read_data[read_index]=c;
			read_index++;
			if (read_index>=read_length) { // that was last byte of payload!
				read_state=STATE_END;
			}
			break;
		case STATE_END: /* end byte */
			{
			read_state=STATE_START;
			p.command=c>>4;
			int checksum=0xf&(read_length+p.command+read_sumpay+(read_sumpay>>4));
			int checkread=0xf&(c);
			if (checkread==checksum) { /* checksum match--valid packet! */
				p.valid=1;
				p.length=read_length;
				p.data=(const unsigned char *)read_data;
			}
			return +1; // let receiver know packet arrived (even if it's bad)
			}
			break;
		default: /* only way to get here is memory corruption!  Reset. */
			read_state=0; break;
		};
		return -1; // no packet was received
	}
private:
	// Private send buffers:
	unsigned char write_data[max_short_length+2]; // short outgoing packets are assembled here

	// Private receiver state:
	unsigned char read_state; // part of message we next expect
	int read_length; // length of payload bytes we expect
	int read_index; // location in payload to next receive
	int read_sumpay; // sum of payload bytes so far
	unsigned char read_data_short[max_short_length]; ///< Short packets go here
	unsigned char *read_data_long; ///< malloc'd long packets, or NULL
	unsigned char *read_data; ///< current target for read data
};



#endif

