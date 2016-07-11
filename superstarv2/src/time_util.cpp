//Mike Moss
//07/10/2016
//Contains time based helper functions...

#include "time_util.hpp"

#include <time.h>

//Gets time in milliseconds...
int64_t millis()
{
	//Seconds...
	int64_t now=time(0);

	//HOPE YOU DIDN'T NEED MILLISECOND RESOLUTION!
	return 1000*now;
}