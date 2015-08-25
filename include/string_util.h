#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <sstream>
#include <string>

template<typename T> std::string to_string(const T& val)
{
	std::ostringstream ostr;
	ostr<<val;
	return ostr.str();
}

std::string to_string(const bool& val);

std::string to_lower(std::string str);

std::string strip_whitespace(std::string str);

#endif