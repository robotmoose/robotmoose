#ifndef TIME_HPP
#define TIME_HPP

#include <time.h>
#include <mongoose/mongoose.h>

//Note, this has second resolution...
inline int64_t millis()
{
	int64_t now=time(0);
	return 1000*now;
}

#endif