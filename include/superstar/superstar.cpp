#include "superstar.hpp"
#include "string_util.hpp"
#include "jsoncpp/json.h"
#include "auth.hpp"
#include "mongoose/mongoose.h"
#include "wget.hpp"
#include <string>
#include "json_util.hpp"
#include <iostream>
// #include <sstream>
// #include <iomanip>

superstar_t::superstar_t(std::string url) : superstar(url)
{
	if(superstar.substr(0,7)!="http://"&&superstar.substr(0,8)!="https://")
	{
		if(superstar.substr(0,9)=="127.0.0.1"||superstar.substr(0,9)=="localhost")
			superstar="http://"+superstar;
		else
			superstar="https://"+superstar;
	}
}

// Gets the value of the path.
//     Calls success_cb on sucess with server response.
//     Calls error_cb on error with the server error object (as per spec).
void superstar_t::get(std::string path, void(*success_cb)(Json::Value), void(*error_cb)(Json::Value)) {
	path = pathify(path);
	Json::Value request = build_skeleton_request("get", path);
	add_request(request, success_cb, error_cb);
}

// Sets path to the value using the given auth.
//     Calls success_cb on success with the server response.
//     Calls error_cb on error with the server error object (as per spec).
void superstar_t::set(std::string path, Json::Value value, std::string auth,
	void(*success_cb)(Json::Value), void(*error_cb)(Json::Value)) {
	path = pathify(path);

	Json::Value temp;
	temp["value"] = value;
	std::string opts = JSON_serialize(temp);
	Json::Value request = build_skeleton_request("set", path, opts);
	request["params"]["auth"]=auth;
	add_request(request, success_cb, error_cb);

}

// Gets sub keys of the given path.
//     Calls success_cb on success with the server response.
//     Calls error_cb on error with the server error object (as per spec).
void superstar_t::sub(std::string path, void(*success_cb)(Json::Value), void(*error_cb)(Json::Value)) {
	path = pathify(path);
	Json::Value request = build_skeleton_request("sub", path);
	add_request(request, success_cb, error_cb);
}

// Pushes the given value onto path using the given auth.
//     Calls success_cb on success with the server response.
//     Calls error_cb on error with the server error object (as per spec).
//     Note, if the path is not an array, it will be after self.
void superstar_t::push(std::string path, Json::Value value, int length, std::string auth,
	void(*success_cb)(Json::Value), void(*error_cb)(Json::Value)) {
	path = pathify(path);
	Json::Value temp;
	temp["value"] = value;
	temp["length"] = length;
	std::string opts = JSON_serialize(temp);
	Json::Value request = build_skeleton_request("push", path, opts);
	request["params"]["auth"]=auth;
	add_request(request, success_cb, error_cb);
}


// Gets the value of path when it changes.
	// Calls success_cb on success with the server response.
	// Calls error_cb on error with the server error object (as per spec).
	// Note, this version is BLOCKING.
void superstar_t::get_next(std::string path, void(*success_cb)(Json::Value), void(*error_cb)(Json::Value)) {
	path = pathify(path);
	Json::Value request = build_skeleton_request("get_next", path);
	request["id"] = 0;
	Json::Value request_error_handler;
	if(error_cb != NULL) {
		request_error_handler["error_cb"] = pointer_to_string(error_cb);
		callbacks[JSON_serialize(request_error_handler["error_cb"])] = error_cb;
	}

	try {
		// Make the request
		std::string data = JSON_serialize(request);
		Json::Value server_response = JSON_deserialize(wget("http://" + superstar + "/superstar/", data));

		// Got an array, must be batch data
		if(server_response.type() == Json::objectValue) {
			// Error Callback
			if( server_response.isMember("error") ) {
				handle_error(request_error_handler, server_response["error"]);
			}
			// Success Callback
			else if(server_response.isMember("result")) {
				if(success_cb != NULL)
					success_cb(server_response["result"]);
			}
		}
		// Server Error
		else
			handle_error(request_error_handler, server_response["error"]);
	}
	catch(std::exception & e) {
		Json::Value error_obj;
		error_obj["code"] = 0;
		error_obj["message"] = e.what();
		handle_error(request_error_handler, error_obj);
	}
}

// Changes auth for the given path and auth to the given value.
//     Calls success_cb on success with the server response.
//     Calls error_cb on error with the server error object (as per spec).
void superstar_t::change_auth(std::string path, Json::Value value, std::string auth,
			void(*success_cb)(Json::Value), void(*error_cb)(Json::Value)) {
	path = pathify(path);
	Json::Value temp;
	temp["value"] = value;
	std::string opts = JSON_serialize(temp);
	Json::Value request = build_skeleton_request("change_auth", path, opts);
	request["params"]["auth"]=auth;
	add_request(request, success_cb, error_cb);
}

// Builds a basic jsonrpc request with given method.
//     Adds path as path and opts as opts to the params object.
//     Note, opts is optional.
Json::Value superstar_t::build_skeleton_request(std::string method, std::string path, std::string opts) {
	path = pathify(path);

	Json::Value request;
	request["jsonrpc"] = "2.0";
	request["method"] = method;
	Json::Value params;
	params["path"] = path;
	request["params"] = params;
	request["id"] = Json::nullValue;
	request["params"]["opts"] = opts;
	return request;
}

// Adds a build request into the batch queue.
//     Note, won't be sent until .flush() is called.
void superstar_t::add_request(Json::Value & request, void(*success_cb)(Json::Value), void(*error_cb)(Json::Value)) {
	Json::Value full_request;
	full_request["request"] = request;

	if(success_cb != NULL) {
		full_request["success_cb"] = pointer_to_string(success_cb);
		callbacks[JSON_serialize(full_request["success_cb"])] = success_cb;
	}
	if(error_cb != NULL) {
		full_request["error_cb"] = pointer_to_string(error_cb);
		callbacks[JSON_serialize(full_request["error_cb"])] = error_cb;
	}

	queue.push_back(full_request);
}

//Replace all multiple slashes with a single slash (hacky...but it works...)
static std::string remove_double_slashes(std::string str)
{
	size_t times=str.size()/2+1;
	for(size_t ii=0;ii<times;++ii)
		str=replace_all(str,"//","/");
	return str;
}

// Replaces multiple slashes in path with a single slash.
//     Removes all leading and trailing slashes.
std::string superstar_t::pathify(std::string path) {
	path = remove_double_slashes(strip(path, "/"));
	return path;
}



// Builds the batch request object and clears out the current queue.
void superstar_t::flush() {
	// No requests, return.
	if(queue.size() == 0)
		return;

	// Build batch of current requests.
	std::vector<Json::Value> batch;
	for(int i=0; i<queue.size(); ++i) {
		std::string path = JSON_serialize(queue[i]["request"]["params"]["path"]);
		std::string opts = JSON_serialize(queue[i]["request"]["params"]["opts"], true);
		queue[i]["request"]["id"] = i;
		if(queue[i]["request"]["params"].isMember("auth")) {
			std::string auth = to_hex_string(JSON_serialize(queue[i]["request"]["params"]["auth"]));
			std::string data = to_hex_string(path+opts);
			queue[i]["request"]["params"]["auth"] =  hmac_sha256(auth, data);
		}
		batch.push_back(queue[i]["request"]);
	}
	old_queue=queue;
	queue.clear();

	try {
		// Make the request
		std::string data = "[";
		for(std::vector<Json::Value>::iterator iter = batch.begin(); iter != batch.end(); ++iter) {
			data += JSON_serialize(*iter);
			if(iter != batch.end() - 1) data += ',';
		}
		data += "]";
		Json::Value server_response = JSON_deserialize(wget("http://" + superstar + "/superstar/", data));

		// Got an array, must be batch data
		if(server_response.type() == Json::arrayValue) {
			for(Json::ValueIterator iter = server_response.begin(); iter != server_response.end(); ++iter) {
				if( !((*iter).isMember("id")) )
					continue;

				// Error Callback
				if( (*iter).isMember("error") ) {
					Json::Value response_obj = (*iter)["error"];
					handle_error(old_queue[strtol(JSON_serialize((*iter)["id"]).c_str(), NULL, 10)], response_obj);
					continue;
				}

				// Success Callback
				if( (*iter).isMember("result") ) {
					Json::Value response_obj = (*iter)["result"];
					if( old_queue[strtol(JSON_serialize((*iter)["id"]).c_str(), NULL, 10)]["success_cb"] != Json::nullValue ) {
						callbacks[JSON_serialize(old_queue[strtol(JSON_serialize((*iter)["id"]).c_str(), NULL, 10)]["success_cb"])](response_obj);
					}
				}
			}
		}
		// Server error ...
		else {
			for(std::vector<Json::Value>::iterator iter = old_queue.begin(); iter != old_queue.end(); ++iter)
				handle_error(*iter, server_response["error"]);
		}
	}
	catch(std::exception & e) {
		Json::Value error_obj;
		error_obj["code"] = 0;
		error_obj["message"] = e.what();

		for(std::vector<Json::Value>::iterator iter = old_queue.begin(); iter != old_queue.end(); ++iter)
			handle_error(*iter, error_obj);
	}
}

void superstar_t::handle_error(Json::Value request, Json::Value error) {
	if(request.isMember("error_cb"))
		callbacks[JSON_serialize(request["error_cb"])](error);
	else
		std::cout << "Superstar error (" << error["code"] << ") - " << error["message"] << std::endl;
}