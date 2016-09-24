//Mike Moss
//09/24/2016
//Contains helper function for JSONCPP.

#include "json_util.hpp"

#include <stdexcept>
#include <sstream>

//Converts json to string (pretty prints with formatting...helpful for debugging...).
std::string JSON_serialize(const Json::Value& json,bool pretty)
{
	//No spaces or newlines.
	if(!pretty)
	{
		Json::FastWriter writer;
		std::string ret=writer.write(json);
		while(ret.size()>0&&isspace(ret[ret.size()-1])!=0)
			ret=ret.substr(0,ret.size()-1);
		return ret;
	}

	//Printed with formatting...helpful for debugging...
	std::ostringstream ostr;
	ostr<<json<<std::endl;
	return ostr.str();
}

//Converts string to json (or throws error).
//  JSONCPP doesn't seem to care about trailing crap at the end of the str...
Json::Value JSON_deserialize(const std::string& str)
{
	Json::Reader reader;
	Json::Value json;
	if(!reader.parse(str,json))
		throw std::runtime_error(reader.getFormattedErrorMessages());
	return json;
}

//Checkes if the given json object is a string.
//  Note, JSONCPP seems to parse
//  everything as a string...so check for everything but a string to get
//  the desired effect....
bool JSON_isString(const Json::Value json)
{
	return !(json.isNull()||
		json.isBool()||
		json.isInt()||
		json.isUInt()||
		json.isIntegral()||
		json.isDouble()||
		json.isNumeric()||
		json.isArray()||
		json.isObject());
}