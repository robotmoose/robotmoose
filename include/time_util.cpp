//Time Utilities Source
//	Created By:		Mike Moss
//	Modified On:	04/11/2014

//Definitions for "time_util.hpp"
#include "time_util.hpp"

//C Standard Library Header
#include <cstdlib>

//Time Header
#include <time.h>

//Windows Dependices
#if(defined(_WIN32)&&!defined(__CYGWIN__))
	#include <stdint.h>

	//Define the Unix Epoch
	#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL

	//Define the Unix Timezone Struct
	#ifndef _TIMEZONE_DEFINED
	#define _TIMEZONE_DEFINED

		struct timezone
		{
				int tz_minuteswest;
				int tz_dsttime;
		};

	#endif

	//Unix Get Time of Day Function (http://www.suacommunity.com/dictionary/gettimeofday-entry.php)
	int gettimeofday(struct timeval* time_value,struct timezone* time_zone)
	{
		//Check For Bad Time Value
		if(time_value==NULL)
			return -1;

		//File Time Variable
		FILETIME file_time;

		//Current Time Variable
		uint64_t current_time=0;

		//Get System Time (Returns in nanoseconds*100)
		GetSystemTimeAsFileTime(&file_time);
		current_time|=file_time.dwHighDateTime;
		current_time<<=32;
		current_time|=file_time.dwLowDateTime;

		//Convert System Time to Microseconds
		current_time/=10;

		//Subtract Epoch
		current_time-=DELTA_EPOCH_IN_MICROSECS;

		//Pack System Time Into Passed Timeval
		time_value->tv_sec=static_cast<long>(current_time/1000000UL);
		time_value->tv_usec=static_cast<long>(current_time%1000000UL);

		//Set Timezone
		if(time_zone!=NULL)
		{
			_tzset();
			(*time_zone).tz_minuteswest=_timezone/60;
			(*time_zone).tz_dsttime=_daylight;
		}

		//Return Success
		return 0;
	}

//Unix Dependices
#else
	#include <sys/time.h>
#endif

//Millis Function (Returns system time in milliseconds)
unsigned long msl::millis()
{
	//Get System Time
	timeval time;
	gettimeofday(&time,NULL);

	//Return System Time in Milliseconds (seconds*1000+microseconds/1000)
	return time.tv_sec*1000+time.tv_usec/1000;
}

//Nano Second Sleep Function (Used for relinquishing time slices)
int msl::nsleep(long nseconds)
{
	timespec tt0,tt1;
	tt0.tv_sec=0;
	tt0.tv_nsec=nseconds;

	return nanosleep(&tt0,&tt1);
}