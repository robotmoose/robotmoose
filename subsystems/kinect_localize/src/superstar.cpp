#include "superstar.hpp"
#include "string_util.hpp"
#include "jsoncpp/json.h"
#include "json_util.hpp"
#include "auth.hpp"
#include "mongoose/mongoose.h"
#include <string>
#include <algorithm>

superstar_t::superstar_t(std::string url) : superstar(url) {}

// Gets the value of the path.
//     Calls successcb on sucess with server response.
//     Calls errorcb on error with the server error object (as per spec).
void superstar_t::get(std::string path, void(*successcb)(Json::Value), void(*errorcb)(Json::Value)) {
	path = pathify(path);
	Json::Value request = build_skeleton_request("get", path);
	add_request(request, successcb, errorcb);
}

// Sets path to the value using the given auth.
//     Calls success_cb on success with the server response.
//     Calls error_cb on error with the server error object (as per spec).
void superstar_t::set(std::string path, Json::Value value, std::string auth, 
	void(*successcb)(Json::Value), void(*errorcb)(Json::Value)) {
	path = pathify(path);

	Json::Value temp;
	temp["value"] = value;
	std::string opts = JSON_serialize(temp);
	Json::Value request = build_skeleton_request("set", path, opts);
	request["params"]["auth"]=auth;
	add_request(request, successcb, errorcb);

}

// Gets sub keys of the given path.
//     Calls success_cb on success with the server response.
//     Calls error_cb on error with the server error object (as per spec).
void superstar_t::sub(std::string path, void(*successcb)(Json::Value), void(*errorcb)(Json::Value)) {
	path = pathify(path);
	Json::Value request = build_skeleton_request("sub", path);
	add_request(request, successcb, errorcb);
}

// Pushes the given value onto path using the given auth.
//     Calls success_cb on success with the server response.
//     Calls error_cb on error with the server error object (as per spec).
//     Note, if the path is not an array, it will be after self.
void superstar_t::push(std::string path, Json::Value value, int length, std::string auth, 
	void(*successcb)(Json::Value), void(*errorcb)(Json::Value)) {
	path = pathify(path);
	Json::Value temp;
	temp["value"] = value;
	temp["length"] = length;
	std::string opts = JSON_serialize(temp);
	Json::Value request = build_skeleton_request("push", path, opts);
	request["params"]["auth"]=auth;
	add_request(request, successcb, errorcb);
}


void superstar_t::get_next(std::string path, void(*successcb)(Json::Value), void(*errorcb)(Json::Value)) {
	path = pathify(path);
	Json::Value request = build_skeleton_request("get_next", path);
	request["id"] = 0;
	Json::Value request_error_handler;
	request_error_handler["error_cb"] = errorcb;

	// try{

	// }
}

// Changes auth for the given path and auth to the given value.
//     Calls success_cb on success with the server response.
//     Calls error_cb on error with the server error object (as per spec).
void superstar_t::change_auth(std::string path, Json::Value value, std::string auth, 
			void(*successcb)(Json::Value), void(*errorcb)(Json::Value)) {
	path = pathify(path);
	Json::Value temp;
	temp["value"] = value;
	std::string opts = JSON_serialize(temp);
	Json::Value request = build_skeleton_request("change_auth", path, opts);
	request["params"]["auth"]=auth;
	add_request(request, successcb, errorcb);
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
void superstar_t::add_request(Json::Value & request, void(*successcb)(Json::Value), void(*errorcb)(Json::Value)) {
	Json::Value full_request;
	full_request["request"] = request;
	full_request["success_cb"] = successcb;
	full_request["error_cb"] = errorcb;
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
		Json::Value request = queue[i]["request"];
		std::string path = request["params"]["path"];
		std::string opts = request["params"]["opts"];
		request["id"] = i;
		if(request["params"].isMember("auth")) {
			std::string auth = to_hex_string(request["params"]["auth"]);
			std::string data = to_hex_string(path+opts);
			request["params"]["auth"] =  hmac_sha256(auth, data);
		}
		batch.push_back(queue[i]["request"]);
	}
	old_queue=queue;
	queue.clear();
}











struct wget_t
{
	std::string data;
	std::string error;
	bool done;
};



static inline void wget_ev_handler(mg_connection* connection,int ev,void* ev_data)
{
	wget_t& responder=*(wget_t*)(connection->mgr->user_data);

	if(ev==MG_EV_CONNECT)
	{
		int status=*(int*)ev_data;

		if(status!=0)
		{
			responder.error=strerror(status);
			responder.done=true;
			return;
		}
	}
	else if(ev==MG_EV_HTTP_REPLY)
	{
		connection->flags|=MG_F_CLOSE_IMMEDIATELY;
		http_message* hm=(http_message*)ev_data;

		if(hm->resp_code!=200)
		{
			responder.error="Connection error: "+std::to_string(hm->resp_code)+".";
			responder.done=true;
			return;
		}

		responder.data=std::string(hm->body.p,hm->body.len);
		responder.done=true;
	}
}

static inline std::string wget(const std::string& address,const std::string& post_data)
{
	wget_t responder;
	responder.data="";
	responder.error="";
	responder.done=false;

	mg_mgr mgr;
	mg_mgr_init(&mgr,&responder);
	mg_connect_http(&mgr,wget_ev_handler,address.c_str(),nullptr,post_data.c_str());

	while(mgr.active_connections!=nullptr)
		mg_mgr_poll(&mgr,1000);

	mg_mgr_free(&mgr);

	if(responder.error.size()>0)
		throw std::runtime_error(responder.error);

	return responder.data;
}