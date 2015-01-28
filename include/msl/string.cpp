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
	size_t found=str.find(find,0);

	while(found!=std::string::npos)
	{
		str.replace(found,find.size(),replace);
		found+=replace.size();
		found=str.find(find,found-find.size()+replace.size());
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