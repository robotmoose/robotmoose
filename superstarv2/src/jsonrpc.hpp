//Mike Moss
//07/21/2016
//Contains an implementation of the jsonrpc 2.0 specification:
//  http://www.jsonrpc.org/specification

#ifndef JSONRPC_HPP
#define JSONRPC_HPP

#include "comet.hpp"
#include <jsoncpp/json.h>
#include <map>
#include <mongoose/mongoose.h>
#include <string>
#include "superstar.hpp"

//Attempts to handle and send the response of the jsonrpc request(s) object in
//  post_data via the passed superstar object.
//  Note, also handles comet connections.
void jsonrpc(superstar_t& superstar,comet_mgr_t& comet_mgr,
	const std::string& post_data,mg_connection* conn);

//Handles individual jsonrpc request via the passed superstar/comet_mgr objects.
Json::Value jsonrpc_handle(superstar_t& superstar,comet_mgr_t& comet_mgr,
	const Json::Value request,std::map<std::string,bool>& changed_paths);

//Builds a jsonrpc error object with given code, message, and data (data is optional).
Json::Value jsonrpc_error(int code,const std::string& message,const std::string& data="");

//Build a skeleton jsonrpc response object (version and id).
Json::Value jsonrpc_skeleton();

//Checkes if the given request's id is a valid one (as in the specification).
//  Note, id is supposed to be a string/number...JSONCPP seems to parse
//  everything as a string...so check for everything but a string to get
//  the desired effect.
bool jsonrpc_invalid_id(const Json::Value id);

//Checkes if the given request's method is supported (as in a get/set/sub/push/get_next).
bool jsonrpc_supported_method(const Json::Value method);

#endif