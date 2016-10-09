//Time Utilities Header
//	Created By:		Mike Moss
//	Modified On:	04/11/2014

//Begin Define Guards
#ifndef MSL_TIME_UTIL_H
#define MSL_TIME_UTIL_H

//Windows Dependices
#if(defined(_WIN32)&&!defined(__CYGWIN__))
	//Windows Header
	#include <windows.h>

	//Unix Get Time of Day Function (http://www.suacommunity.com/dictionary/gettimeofday-entry.php)
	int gettimeofday(struct timeval* time_value,struct timezone* time_zone);
#else
	//Header for usleep
	#include <unistd.h>
#endif

//MSL Namespace
namespace msl
{
	//Get System Time in Milliseconds
	unsigned long millis();

	//Nano Second Sleep Function (Used for relinquishing time slices)
	int nsleep(long nseconds);
}

//End Define Guards
#endif