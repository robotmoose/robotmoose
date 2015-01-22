#include "time.hpp"

#include <chrono>
#include <thread>

std::int64_t msl::nanos()
{
	auto system_time=std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(system_time).count();
}

std::int64_t msl::micros()
{
	auto system_time=std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::microseconds>(system_time).count();
}

std::int64_t msl::millis()
{
	auto system_time=std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(system_time).count();
}

void msl::delay_ns(const std::int64_t nanoseconds)
{
	std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
}

void msl::delay_us(const std::int64_t microseconds)
{
	std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void msl::delay_ms(const std::int64_t milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}