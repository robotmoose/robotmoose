#include "string.hpp"

#include <algorithm>
#include <ctype.h>
#include <iomanip>
#include <sstream>

std::string msl::to_lower(std::string str)
{
	std::transform(str.begin(),str.end(),str.begin(),tolower);
	return str;
}

std::string msl::to_upper(std::string str)
{
	std::transform(str.begin(),str.end(),str.begin(),toupper);
	return str;
}

std::string msl::to_hex_string(const std::string& str)
{
	std::ostringstream ostr;
	ostr<<"0x";

	for(auto ii:str)
		ostr<<std::hex<<std::setw(2)<<std::setfill('0')<<((int)ii&0x000000ff);

	return ostr.str();
}

std::string msl::replace_all(std::string str,const std::string& find,const std::string& replace)
{
	size_t pos=0;

	while((pos=str.find(find,pos))!=std::string::npos)
	{
		str.replace(pos,find.size(),replace);
		pos+=replace.size();
	}

	return str;
}

bool msl::ends_with(const std::string& str,const std::string& suffix)
{
	return (str.size()>=suffix.size()&&str.compare(str.size()-suffix.size(),suffix.size(),suffix)==0);
}

bool msl::starts_with(const std::string& str,const std::string& prefix)
{
	return (str.size()>=prefix.size()&&str.compare(0,prefix.size(),prefix)==0);
}