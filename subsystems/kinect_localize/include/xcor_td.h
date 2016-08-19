#ifndef XCOR_TD_INCLUDED
#define XCOR_TD_INCLUDED

#include <stdint.h> // for int32_t
#include <utility> // for std::pair

std::pair<int64_t, int32_t> xcor_td(int32_t * signal1, int32_t * signal2, const int & NUMSAMPLES_XCOR, const int & XCOR_WIDTH);

#endif // XCOR_TD_INCLUDED