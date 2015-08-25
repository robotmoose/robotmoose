#include "string_util.h"

#include <cctype>

std::string to_string(const bool& val)
{
	std::ostringstream ostr;

	if(val)
		ostr<<"true";
	else
		ostr<<"false";

	return ostr.str();
}

std::string to_lower(std::string str)
{
	for(size_t ii=0;ii<str.size();++ii)
		str[ii]=tolower(str[ii]);

	return str;
}

std::string strip_whitespace(std::string str)
{
	while(str.size()>0&&isspace(str[0])!=0)
		str=str.substr(1,str.size()-1);

	while(str.size()>0&&isspace(str[str.size()-1])!=0)
		str=str.substr(0,str.size()-1);

	return str;
}