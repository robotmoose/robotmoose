#ifndef MSL_C11_JSON_HPP
#define MSL_C11_JSON_HPP

#include "jsoncpp/json.h"
#include <string>

namespace msl
{
	typedef Json::Value json;
	std::string serialize(const json& json);
	json deserialize(const std::string& str);
}

#endif