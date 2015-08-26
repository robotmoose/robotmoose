#include "string_util.h"

#include <cctype>
#include <stdexcept>

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

bool to_bool(const std::string& str)
{
	if(is_bool(str))
		return (str!="false"&&str!="");

	throw std::runtime_error("String \""+str+"\" is not a bool.");
}

int to_int(const std::string& str)
{
	if(is_int(str))
		return atoi(str.c_str());

	throw std::runtime_error("String \""+str+"\" is not an int.");
}

double to_double(const std::string& str)
{
	if(is_double(str))
		return atof(str.c_str());

	throw std::runtime_error("String \""+str+"\" is not a double.");
}

bool is_bool(std::string str)
{
	str=strip_whitespace(to_lower(str));
	return (str=="false"||str=="true"||str=="");
}

bool is_int(std::string str)
{
	str=strip_whitespace(str);

	if(str.size()>0&&str[0]=='-')
		str=str.substr(1,str.size()-1);

	if(str.size()==0)
		return false;

	for(size_t ii=0;ii<str.size();++ii)
		if(isdigit(str[ii])==0)
			return false;

	if(str.size()>1&&str[0]<='0')
		return false;

	return true;
}

bool is_double(std::string str)
{
	str=strip_whitespace(str);

	if(str.size()>0&&(str[0]=='-'||str[0]=='.'))
		str=str.substr(1,str.size()-1);

	if(str.size()==0)
		return false;

	size_t dot_count=0;
	size_t count_to_first_dot=0;

	for(size_t ii=0;ii<str.size();++ii)
	{
		if(isdigit(str[ii])==0&&str[ii]!='.')
			return false;

		if(str[ii]=='.')
			++dot_count;
		else if(dot_count==0)
			++count_to_first_dot;

		if(dot_count>1)
			return false;

		if(dot_count==1&&count_to_first_dot>1&&str[0]=='0')
			return false;
	}

	if(dot_count==0&&str.size()>1&&str[0]<='0')
		return false;

	return true;
}