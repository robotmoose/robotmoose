#include "string.hpp"

#include <algorithm>
#include <cctype>
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

std::string msl::from_hex_string(std::string str)
{
	std::string ret="";

	if(msl::starts_with(str,"0x"))
		str=str.substr(2,str.size()-2);

	if(str.size()%2!=0)
		str.insert(0,"0");

	size_t pos=0;
	std::string temp_byte;

	while(pos<str.size())
	{
		temp_byte+=str[pos];

		if(temp_byte.size()>=2||pos+1==str.size())
		{
			ret+=std::stoi("0x"+temp_byte,0,16);
			temp_byte="";
		}

		++pos;
	}

	return ret;
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
	return (str.size()>=suffix.size()&&starts_with(str.substr(str.size()-suffix.size(),suffix.size()),suffix));
}

bool msl::starts_with(const std::string& str,const std::string& prefix)
{
	return (str.size()>=prefix.size()&&str.compare(0,prefix.size(),prefix)==0);
}