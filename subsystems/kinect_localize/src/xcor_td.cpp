#include "xcor_td.h"
#include <stdint.h> // for int32_t
#include <utility> // for std::pair
#include <cstring> // for memset and memcpy.
#include <string>

std::pair<int64_t, int32_t> xcor_td(int32_t * signal1, int32_t * signal2, const int & NUMSAMPLES_XCOR, const int & XCOR_WIDTH) {
	// Zero-pad the second input signal
	int32_t * signal2_ext = new int32_t [NUMSAMPLES_XCOR+XCOR_WIDTH];
	memset(signal2_ext, 0, XCOR_WIDTH/2*sizeof(int32_t));
	memcpy(signal2_ext+XCOR_WIDTH/2, signal2, NUMSAMPLES_XCOR*sizeof(int32_t));
	memset(signal2_ext+XCOR_WIDTH/2+NUMSAMPLES_XCOR, 0, XCOR_WIDTH/2*sizeof(int32_t));

	// Calculate the correlation of the signals
	std::pair<int64_t, int32_t> xcor_results(0,0);
	for(int i=0; i<XCOR_WIDTH; ++i) {
		int64_t sum=0;
		for(int j=0; j<NUMSAMPLES_XCOR; ++j) {
			sum += (int64_t)signal1[j] * (int64_t)signal2_ext[j];
		}
		if(xcor_results.first < sum) {
			xcor_results.first = sum;
			// Signal is sliding backwards so lag is negative
			xcor_results.second = -(i-XCOR_WIDTH/2);
		}
		++signal2_ext;
	}
	delete [] (signal2_ext-XCOR_WIDTH);
	return xcor_results;
}