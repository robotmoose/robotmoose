//Mike Moss
//07/09/2016
//Contains string based helper functions (inspired by python).

#include "string_util.hpp"

#include <iomanip>
#include <sstream>

//Returns whether str starts with prefix.
bool starts_with(const std::string& str,const std::string& prefix)
{
	return (str.size()>=prefix.size()&&str.compare(0,prefix.size(),prefix)==0);
}

//Returns whether str ends with suffix.
bool ends_with(const std::string& str,const std::string& suffix)
{
	return (str.size()>=suffix.size()&&starts_with(str.substr(str.size()-suffix.size(),suffix.size()),suffix));
}

//Strips contiguous blocks of match from start of str.
std::string strip_start(std::string str,const std::string& match)
{
	while(starts_with(str,match))
		str=str.substr(match.size(),str.size());
	return str;
}

//Strips contiguous blocks of match from end of str.
std::string strip_end(std::string str,const std::string& match)
{
	while(ends_with(str,match))
		str=str.substr(0,str.size()-match.size());
	return str;
}

//Strips contiguous blocks of match from start and end of str.
std::string strip(std::string str,const std::string& match)
{
	return strip_end(strip_start(str,match),match);
}

//Replace all instances of find in str with replace.
std::string replace_all(std::string str,const std::string& find,const std::string& replace)
{
	size_t pos=0;
	while((pos=str.find(find,pos))!=std::string::npos)
	{
		str.replace(pos,find.size(),replace);
		pos+=replace.size();
	}
	return str;
}

//Splits str into tokens separated by delim.
//  If str is "", then no tokens are returned.
//  If delim is "" and str isn't "" then one token is returned (str).
//  Trailing and ending tokens add in empty tokens:
//    So str==",1,2," and delim=="," would yield ["","1","2",""].
std::vector<std::string> split(const std::string& str,const std::string& delim)
{
	std::vector<std::string> tokens;

	//String empty, no tokens.
	if(str.size()==0)
		return tokens;

	//Delimeter empty, return string.
	if(delim.size()==0)
	{
		tokens.push_back(str);
		return tokens;
	}

	//Get the tokens...
	size_t pos=0;
	size_t old_pos=0;
	while((pos=str.find(delim,pos))!=std::string::npos)
	{
		tokens.push_back(str.substr(old_pos,pos-old_pos));
		pos+=delim.size();
		old_pos=pos;
	}

	//Don't forget the last token.
	tokens.push_back(str.substr(old_pos,str.size()-old_pos));

	return tokens;
}

//Converts bytes of str into a hex string.
std::string to_hex_string(const std::string& str)
{
	std::ostringstream ostr;
	for(size_t ii=0;ii<str.size();++ii)
		ostr<<std::hex<<std::setw(2)<<std::setfill('0')<<((int)str[ii]&0x000000ff);
	return ostr.str();
}