#include "json.hpp"

#include <sstream>

std::string msl::serialize(const msl::json& json)
{
	Json::FastWriter writer;
	std::string ret=writer.write(json);

	while(ret.size()>0&&isspace(ret[ret.size()-1])!=0)
		ret.pop_back();

	return ret;
}

msl::json msl::deserialize(const std::string& str)
{
	std::istringstream istr(str);
	msl::json json;
	istr>>json;
	return json;
}