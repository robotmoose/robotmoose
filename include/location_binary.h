/**
  Exports camera location data from aruco camera viewer, using a simple flat binary file.

Dr. Orion Lawlor, lawlor@alaska.edu, 2014-03.  Public Domain.
*/
#ifndef __LOCATION_BINARY_H
#define __LOCATION_BINARY_H
#include <stdio.h> /* FILE and fopen */
#include <stdint.h>

class location_binary {
public:
	uint32_t valid; // 1 if detected, 0 if did not detect

	// Location of the center of the camera, in meters
	float x; // right from marker
	float y; // out from marker
	float z; // down from marker
	
	// Camera's pointing angle in the X-Y plane, degrees from Y axis
	float angle;
	
	uint32_t count; // incremented at each update (to detect crash/lag/invisible)
	uint32_t vidcap_count; // incremented at each update
	uint32_t marker_ID; // marker's ID number

	location_binary() {
		valid=0;
		x=y=z=angle=0.0f;
		count=0;
		vidcap_count=0;
	}
};

/**
  Reads binary location file
*/
class location_reader {
	location_binary bin;
public:
	location_reader() {bin.count=0;}
	
	/** Return true if the file at this path has been updated. */
	bool updated(const char *bin_path,location_binary &bin_out) {
		FILE *fbin=fopen(bin_path,"rb+");
		if (fbin==NULL) { // file doesn't exist (yet)
			return false;
		} 
		location_binary bin_new;
		if (1!=fread(&bin_new,sizeof(bin_new),1,fbin)) { // atomic(?) file read
			return false;
		}
		fclose(fbin);
		if (bin_new.count!=bin.count) {
			bin=bin_new;
			bin_out=bin;
			return true;
		} /* else */
		return false;
	}
};

#endif

