#include <algorithm> /* for std::sort */
typedef int (*timeable_fn)(void);

/**
  Return the current time in seconds (since, well, something or other!).
*/
#if defined(_WIN32)
#  include <sys/timeb.h>
#  define time_in_seconds_granularity 0.1 /* seconds */
double time_in_seconds(void) { /* This seems to give terrible resolution (60ms!) */
        struct _timeb t;
        _ftime(&t);
        return t.millitm*1.0e-3+t.time*1.0;
}
#else /* UNIX or other system */
#  include <sys/time.h> //For gettimeofday time implementation
#  define time_in_seconds_granularity 0.005 /* seconds */
double time_in_seconds(void) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec*1.0e-6+tv.tv_sec*1.0;
}
#endif

/**
  Return the number of seconds this function takes to run.
  May run the function several times (to average out 
  timer granularity).
*/
double time_function_onepass(timeable_fn fn)
{
	unsigned int i,count=1;
	double timePer=0;
	for (count=1;count!=0;count*=2) {
		double start, end, elapsed;
		start=time_in_seconds();
		for (i=0;i<count;i++) fn();
		end=time_in_seconds();
		elapsed=end-start;
		timePer=elapsed/count;
		if (elapsed>time_in_seconds_granularity) /* Took long enough */
			return timePer;
	}
	/* woa-- if we got here, "count" reached integer wraparound before 
	  the timer ran long enough.  Return the last known time */
	return timePer;
}

/**
  Return the number of seconds this function takes to run.
  May run the function several times (to average out 
  timer granularity).
*/
double time_function(timeable_fn fn)
{
	enum {
#if defined(_WIN32) /* Win32 timer has coarse granularity--too slow otherwise! */
		ntimes=1
#else
		ntimes=3
#endif
	};
	double times[ntimes];
	for (int t=0;t<ntimes;t++)
		times[t]=time_function_onepass(fn);
	std::sort(&times[0],&times[ntimes]);
	return times[ntimes/2];
}

/**
  Time a function's execution, and print this time out.
*/
void print_time(const char *fnName,timeable_fn fn)
{
	double sec=time_function(fn);
	printf("%s: ",fnName);
	if (1 || sec<1.0e-6) printf("%.2f ns/call\n",sec*1.0e9);
	else if (sec<1.0e-3) printf("%.2f us/call\n",sec*1.0e6);
	else if (sec<1.0e0) printf("%.2f ms/call\n",sec*1.0e3);
	else printf("%.2f s/call\n",sec);
}
