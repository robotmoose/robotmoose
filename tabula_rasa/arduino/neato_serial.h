/**
  Neato XV-11 Laser Distance Sensor (LDS) serial data formatting code.
  This version does most of the protocol decoding on the Arduino side.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-02-19 (Public Domain)
*/
#ifndef __CYBERALASKA__NEATO_SERIAL__H
#define __CYBERALASKA__NEATO_SERIAL__H

/*********** Code shared with Arudino *********************/

/** Stores sample data in one direction.
  It's a bitfield so it fits in 16 bits, to save serial comm bandwidth. */
class NeatoLDSdir {
public:
	unsigned short distance:13; // Distance from sensor to target in mm, or 0 if invalid
	unsigned short signal:3; // Reflected signal strength (log base 4), or 0 if invalid
	//unsigned short quality:2; // 0 for bad; 1 for warning; 2 for good

	inline void clear(void) {
		distance=signal=0;
	}
};

/** A batch of directional sample data.  We keep batches on the arduino,
instead of the whole 360 degree circle, to save Arduino memory space
and to reduce communication latency.  The "batch" formats the Aruduino-to-PC
serial data protocol.
*/
class NeatoLDSbatch {
public:
	enum {size=12}; // number of reports to accumulate at one time (multiple of 4, evenly divides 360)
	short index:10; // starting angle, from 0-359 degrees.  -1 for invalid.
	unsigned short errors:6; // Neato serial communication errors while receiving this batch
	unsigned short speed64; // RPM*64
	NeatoLDSdir dir[size]; // directions, starting at index.

	NeatoLDSbatch() { clear();}
	void clear(void) {
		index=-1; errors=0; speed64=0;
		for (int i=0;i<size;i++) dir[i].clear();
	}
};




/**
  This class parses the raw serial output from the Neato XV-11 Laser Distance Sensor,
  and stores the output as a set of distances in directions.  It's typically run
  on the Arduino side, and the batches are sent up to the PC.
*/
template <class SerialPort>
class NeatoLDS {
public:
	NeatoLDSbatch curBatch; // data currently being received (overwritten)
	NeatoLDSbatch lastBatch; // last complete set of data received

	SerialPort &p;
private:
	int index; // current angle being received (degrees)
	int state; // current byte being received
	int reportByte; // next byte of report to be received [0-4]
	unsigned int reportDistance, reportSignal; // current report fields
	NeatoLDSdir report; // current report being accumulated
	NeatoLDSdir reports[4]; // current data packet being accumulated
	unsigned int dataLow; // 1: on low byte; 0: on high byte
	unsigned short data16; // 16-bit data accumulated
	unsigned long check32; // checksum data accumulated so far
public:
	NeatoLDS(SerialPort &port)
		:p(port)
	{
		state=0; data16=check32=0; dataLow=1;
		reportByte = 0; reportDistance = 0; reportSignal = 0; index = 0;
	}

	bool read(void) {
		bool read_data=false;
		// See protocol at http://xv11hacking.wikispaces.com/LIDAR+Sensor
		if (p.available()) {
			int c=p.read();
			if (c==-1) return false; // why was it available, then?
			read_data=true;

			if (dataLow) { // starting new low byte
				data16=c;
				dataLow=0;
			} else { // high byte
				data16=data16|(c<<8);
				if (state!=9)
					check32=(check32<<1)+data16;
				dataLow=1;
			}

			switch (state) {
			case 0: // sync byte
				if (c==0xFA) { // valid sync byte
					check32=0;
					data16=c;
					dataLow=0;
					reportByte=0;
					state++;
				} break;
			case 1: // index byte
				if (c>=0xA0 && c<=0xf9) { // valid index
					index=4*(c-0xA0);
					if (curBatch.index<0) { // first index of a batch:
						curBatch.index=index;
					}
					state++;
				}
				else {
					curBatch.errors++;
					state=0; // read error of some kind--start over
				}
				break; // index
			case 2: state++; break; // low byte of speed
			case 3: curBatch.speed64=data16; state++; break; // high byte of speed
			case 4: case 5: case 6: case 7: // main payload
			{ // read data
				switch(reportByte) {
				case 0: break;
				case 1: reportDistance=data16; break;
				case 2: break;
				case 3: reportSignal=data16; break;
				}
				reportByte++;
				if (reportByte==4) {
					// Check for flags in the distance field
					const int invalid_flag=(1<<15);
					const int warning_flag=(1<<14);
					//int reportQuality=2;
					if (reportDistance&invalid_flag) { // invalid data
						reportDistance=0;
						//reportQuality=0;
					}
					if (reportDistance&warning_flag) { // warning flag
						reportDistance&=warning_flag-1; // keep low bits
						//reportQuality=1;
					}

					// Convert signal reflectance to bit count
					unsigned int bits=0;
					while (reportSignal>0) {
  					   reportSignal=reportSignal>>2;
					   bits++;
					}
					if (bits>7) bits=7;
					reportSignal=bits;
/*
					if (val>=(1<<(7*2))) reportSignal=7;
					else {
					   while (val<reportSignal) { bits++; val=val<<2; }
					   reportSignal=bits;
					}
*/

					report.distance=reportDistance;
					report.signal=reportSignal;
					// report.quality=reportQuality;

					// Store report into list
					reports[state-4]=report;
					reportByte=0;
					state++;
				}
				break;
			}
			case 8: state++; break; // first checksum byte
			case 9: { // checksum is complete
				state=0;
				// validate our checksum
				int target=((check32) + (check32>>15))&0x7FFF;
				if (target!=data16)
				{  // checksum does not match: skip copy and warn
					curBatch.errors++;
				}
				else
				{ // valid checksum--copy stored reports to output
					for (int d=0;d<4;d++)
					{
						int i=index+d-curBatch.index;
						if (i>=0 && i<NeatoLDSbatch::size)
							curBatch.dir[i]=reports[d];
					}
				}

				if (((index+4)%NeatoLDSbatch::size)==0)
				{ // that was the last packet in a batch:
					lastBatch=curBatch;
					curBatch.clear();
				}

				}
				break;
			}
		}
		return read_data;
	}

};




/**************** End code shared with Arduino ***********/


#endif

