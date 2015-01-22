#ifndef MSL_C11_STRING_HPP
#define MSL_C11_STRING_HPP

#include <string>

namespace msl
{
	std::string to_lower(std::string str);
	std::string to_upper(std::string str);

	std::string to_hex_string(const std::string& str);

	std::string replace_all(std::string str,const std::string& find,const std::string& replace);

	bool ends_with(const std::string& str,const std::string& suffix);
	bool starts_with(const std::string& str,const std::string& prefix);
}

#endif