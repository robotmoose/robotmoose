//Mike Moss
//07/09/2016
//Contains an implementation of the jsonrpc 2.0 specification: http://www.jsonrpc.org/specification

#ifndef JSONRPC_HPP
#define JSONRPC_HPP

#include <jsoncpp/json.h>
#include <string>
#include "superstar.hpp"

//Attempts to handle and return the response of the jsonrpc request(s) object in
//  post_data via the passed superstar object.
std::string jsonrpc(superstar_t& superstar,const std::string& post_data);

//Handles individual jsonrpc request via the passed superstar object.
Json::Value jsonrpc_handle(superstar_t& superstar,const Json::Value request);

//Builds a jsonrpc error object with given code, message, and data (data is optional).
Json::Value jsonrpc_error(int code,const std::string& message,const std::string& data="");

//Build a skeleton jsonrpc response object (version and id).
Json::Value jsonrpc_skeleton();

//Checkes if the given request's id is a valid one (as in the specification).
//  Note, id is supposed to be a string/number...JSONCPP seems to parse
//  everything as a string...so check for everything but a string to get
//  the desired effect.
bool jsonrpc_invalid_id(const Json::Value request);

//Checkes if the given request's method is a valid one (as in the specification).
//  Note, same mentality here as in the invalid id function, need a string.
bool jsonrpc_invalid_method(const Json::Value request);

//Checkes if the given request's method is supported (as in a get/set/sub/push).
bool jsonrpc_supported_method(const Json::Value request);

#endif