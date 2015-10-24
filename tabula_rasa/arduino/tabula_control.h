/**
 Scalable command and sense system for robot control:

command: PC sending control commands for motor controllers, 
  enable bits, etc.

sensor: Arduino sending temperature, ultrasonic, bumpers, etc.

Currently we just have one single huge command and sensor storage buffer.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-18 (Public Domain)
*/
#ifndef __TABULA_CONTROL_H
#define __TABULA_CONTROL_H

#ifdef __AVR
#  include <Arduino.h>
#endif

#include <stdlib.h> /* for realloc */

/// This array stores all our sensor or command data,
///  stored contiguously and allocated with realloc.
///  Contiguous storage is designed to make it easier to programmatically change.
class tabula_control_storage {
public:
	typedef unsigned char storage_t;
	storage_t *array; /// < contiguous brick of sensor or command data
	int count; /// number of used entries in array
	
	tabula_control_storage() :array(0), count(0) {}
	~tabula_control_storage() { clear(); }
	
	// Zero all data in this buffer
	void zero(void) {
		memset(array,sizeof(storage_t)*count,0);
	}

	// Deallocate all space used in this storage object
	void clear(void) {
		free(array); array=0; count=0; 
	}
	
	/// Return the index of a new allocation of this size
	template <class T>
	int allocate(T startValue=T()) {
		int index=count;
		count+=(sizeof(T)+sizeof(storage_t)-1)/sizeof(storage_t);
		array=(storage_t *)realloc(array,count);
		*(T *)&array[index] = startValue;
		return index;
	}

#ifdef __AVR
	/// For debugging, print the whole array as this datatype.
	template <class T>
	void print() {
		T *arr=(T *)array;
		for (int i=0;i<count/sizeof(T);i++) {
			Serial.print(i*sizeof(T));
			Serial.print(": ");
			Serial.print((long)arr[i]);
			Serial.print(" ");
		}
		Serial.println();
	}
#endif

private: // do NOT copy or assign these objects
	tabula_control_storage(const tabula_control_storage &src);
	void operator=(const tabula_control_storage &src);
};

/// All sensor and command data is stored in these arrays.
extern tabula_control_storage tabula_sensor_storage;
extern tabula_control_storage tabula_command_storage;

// Mark class as not being copy or assignable
class tabula_nocopy {
public:
	tabula_nocopy() {}
private:
	tabula_nocopy(const tabula_nocopy &src);
	void operator=(const tabula_nocopy &src);
};

/**
 This represents one value of type T of sensor data.
*/
template <class T>
class tabula_sensor : public tabula_nocopy {
	int index;
public:
	/// Reserve our index in the global sensor storage array.
	tabula_sensor(T startValue=T()) {
		index=tabula_sensor_storage.allocate<T>(startValue);
	}
	
	/// Return our index in the global array.
	int get_index(void) const { return index; }
	
	/// Get read/write access to the data at this sensor index.
	inline T &get(void) { 
		return *(T *)&tabula_sensor_storage.array[index]; 
	}
	
	/// Convenience helper: support assignment directly to the underlying sensor data.
	T operator=(const T&src) { get()=src; return get(); }
};

/**
 This represents one command value of type T.
*/
template <class T>
class tabula_command : public tabula_nocopy {
	int index;
public:
	/// Reserve our index in the global array.
	tabula_command(T startValue=T()) {
		index=tabula_command_storage.allocate<T>(startValue);
	}
	
	/// Return our index in the global array.
	int get_index(void) const { return index; }
	
	/// Get read/write access to the data at this command index.
	inline T &get(void) { 
		return *(T *)&tabula_command_storage.array[index]; 
	}
	
	/// Convenience helper: support reading directly from the underlying sensor data.
	operator T() { return get(); }
};

#endif

