#include "web_util.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>

//URL encodes the given string.
//  As per: https://en.wikipedia.org/wiki/Percent-encoding
std::string url_encode(const std::string& str)
{
	std::ostringstream encoded;
	encoded.fill('0');
	for(size_t ii=0;ii<str.size();++ii)
	{
		if(isalnum(str[ii])||str[ii]=='-'||str[ii]=='_'||str[ii]=='.'||str[ii]=='~')
			encoded<<std::nouppercase<<str[ii];
		else
			encoded<<std::hex<<std::uppercase<<'%'<<std::setw(2)<<int((unsigned char)str[ii]);
	}
	return encoded.str();
}