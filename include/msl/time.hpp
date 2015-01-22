#ifndef MSL_C11_TIME_HPP
#define MSL_C11_TIME_HPP

#include <cstdint>

namespace msl
{
	std::int64_t nanos();
	std::int64_t micros();
	std::int64_t millis();

	void delay_ns(const std::int64_t nanoseconds);
	void delay_us(const std::int64_t microseconds);
	void delay_ms(const std::int64_t milliseconds);
}

#endif