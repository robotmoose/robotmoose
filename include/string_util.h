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

bool to_bool(const std::string& str);

int to_int(const std::string& str);

double to_double(const std::string& str);

bool is_bool(std::string str);

bool is_int(std::string str);

bool is_double(std::string str);

#endif