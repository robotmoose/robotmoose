#ifndef STRING_UTIL_HPP
#define STRING_UTIL_HPP

#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

inline std::string to_json_array(const std::vector<std::string>& list)
{
	std::string json="[";

	for(size_t ii=0;ii<list.size();++ii)
	{
		json+="\""+list[ii]+"\"";

		if(ii+1<list.size())
			json+=",";
	}

	json+="]";

	return json;
}

inline std::string to_string(const size_t value)
{
	char buffer[100];
	memset(buffer,0,100);
	snprintf(buffer,100,"%zx",value);
	return std::string(buffer);
}

inline std::string to_string(const long value)
{
	char buffer[100];
	memset(buffer,0,100);
	snprintf(buffer,100,"%ld",value);
	return std::string(buffer);
}

inline std::string to_string(const float value)
{
	char buffer[100];
	memset(buffer,0,100);
	snprintf(buffer,100,"%f",value);
	return std::string(buffer);
}

#endif
