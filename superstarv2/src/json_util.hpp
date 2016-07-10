//Mike Moss
//07/09/2016
//Contains helper function for JSONCPP.

#ifndef JSON_UTIL_HPP
#define JSON_UTIL_HPP

#include <jsoncpp/json.h>
#include <string>

//Converts json to string (pretty prints with formatting...helpful for debugging...).
std::string JSON_serialize(const Json::Value& json,bool pretty=false);

//Converts string to json (or throws error).
Json::Value JSON_deserialize(const std::string& str);

//Checkes if the given json object is a string.
//  Note, JSONCPP seems to parse
//  everything as a string...so check for everything but a string to get
//  the desired effect....
bool JSON_isString(const Json::Value json);

#endif