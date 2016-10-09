//Mike Moss
//09/24/2016
//Contains client code to get requests from a superstar server.

#include "superstar.hpp"

#include "auth.hpp"
#include "json_util.hpp"
#include "string_util.hpp"
#include "wget.hpp"

superstar_t::superstar_t(const std::string& url):superstar(url)
{
	if(superstar.substr(0,7)!="http://"&&superstar.substr(0,8)!="https://")
	{
		if(superstar.substr(0,9)=="127.0.0.1"||superstar.substr(0,9)=="localhost")
			superstar="http://"+superstar;
		else
			superstar="https://"+superstar;
	}
}

//Gets the value of path.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
void superstar_t::get(std::string path,success_cb_t success_cb,error_cb_t error_cb)
{
	path=pathify(path);
	Json::Value request=build_skeleton_request("get",path);
	add_request(request,success_cb,error_cb);
}

//Sets path to the value using the given auth.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
void superstar_t::set(std::string path,Json::Value value,const std::string& auth,success_cb_t success_cb,error_cb_t error_cb)
{
	path=pathify(path);
	Json::Value opts;
	opts["value"]=value;
	Json::Value request=build_skeleton_request("set",path,opts);
	build_auth(path,request,auth);
	add_request(request,success_cb,error_cb);
}

//Gets sub keys of the given path.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
void superstar_t::sub(std::string path,success_cb_t success_cb,error_cb_t error_cb)
{
	path=pathify(path);
	Json::Value request=build_skeleton_request("sub",path);
	add_request(request,success_cb,error_cb);
}

//Pushes the given value onto path using the given auth.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
//  Note, if the path is not an array, it will be after this.
void superstar_t::push(std::string path,Json::Value value,const size_t& length,const std::string& auth,success_cb_t success_cb,error_cb_t error_cb)
{
	path=pathify(path);
	Json::Value opts;
	opts["value"]=value;
	opts["length"]=Json::UInt64(length);
	Json::Value request=build_skeleton_request("push",path,opts);
	build_auth(path,request,auth);
	add_request(request,success_cb,error_cb);
}

//Gets the value of path when it changes.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
//  Note, python version is unique because it is BLOCKING.
void superstar_t::get_next(std::string path,const std::string& last_hash,
	success_cb_t success_cb,error_cb_t error_cb)
{
	path=pathify(path);
	Json::Value request=build_skeleton_request("get_next",path);
	request["id"]=0;
	request["params"]["last_hash"]=last_hash;

	try
	{
		//Make the request.
		std::string data=JSON_serialize(request);
		std::string server_response=wget(superstar+"/superstar/",data);

		//Parse response, call responses.
		Json::Value response=JSON_deserialize(server_response);

		//Got an object, must be single request...
		if(response.isObject())
		{
			//Error callback...
			if(response.isMember("error"))
				handle_error(error_cb,response["error"]);

			//Success callback...
			else if(response.isMember("result")&&success_cb!=NULL)
				success_cb(response["result"]);
		}

		//Server error...
		else
		{
			handle_error(error_cb,response["error"]);
		}
	}
	catch(std::exception& error)
	{
		Json::Value error_obj;
		error_obj["code"]=0;
		error_obj["message"]=error.what();
		handle_error(error_cb,error_obj);
	}
	catch(...)
	{
		Json::Value error_obj;
		error_obj["code"]=0;
		error_obj["message"]="Unknown error.";
		handle_error(error_cb,error_obj);
	}
}

//Changes auth for the given path&&auth to the given value.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
void superstar_t::change_auth(std::string path,Json::Value value,const std::string& auth,success_cb_t success_cb,error_cb_t error_cb)
{
	path=pathify(path);
	Json::Value opts;
	opts["value"]=value;
	Json::Value request=build_skeleton_request("change_auth",path,opts);
	build_auth(path,request,auth);
	add_request(request,success_cb,error_cb);
}

//Replaces multiple slashes in path with a single slash.
//  Removes all leading&&trailing slashes.
std::string superstar_t::pathify(std::string path)
{
	size_t times=path.size()/2+1;
	for(size_t ii=0;ii<times;++ii)
		path=replace_all(path,"//","/");
	while(path.size()>0&&path[0]=='/')
		path=path.substr(1,path.size()-1);
	while(path.size()>0&&path[path.size()-1]=='/')
		path=path.substr(0,path.size()-1);
	return path;
}

//Builds a basic jsonrpc request with given method.
//  Adds path as path&&opts as opts to the params object.
//  Note, opts is optional.
Json::Value superstar_t::build_skeleton_request(const std::string& method,std::string path,Json::Value opts)
{
	path=pathify(path);
	Json::Value request;
	request["jsonrpc"]="2.0";
	request["method"]=method;
	Json::Value params;
	params["path"]=path;
	params["opts"]=JSON_serialize(opts);
	request["params"]=params;
	return request;
}

//Builds HMACSHA256 for given path and request object with given auth.
void superstar_t::build_auth(std::string& path,Json::Value& request,const std::string& auth)
{
	path=pathify(path);
	request["params"]["auth"]=auth;
}

//Adds a build request into the batch queue.
//  Note, won't be sent until .flush() is called.
void superstar_t::add_request(Json::Value& request,success_cb_t success_cb,success_cb_t error_cb)
{
	starequest_t starequest;
	starequest.request=request;
	starequest.success_cb=success_cb;
	starequest.error_cb=error_cb;
	queue.push_back(starequest);
}

//Builds the batch request object and clears out the current queue.
void superstar_t::flush()
{
	//No requests, return.
	if(queue.size()==0)
		return;

	//Build batch of current requests.
	Json::Value batch=Json::arrayValue;
	for(size_t ii=0;ii<queue.size();++ii)
	{
		Json::Value& request=queue[ii].request;
		std::string path=request["params"]["path"].asString();
		std::string opts=request["params"]["opts"].asString();
		request["id"]=Json::UInt64(ii);
		if(request["params"].isMember("auth"))
		{
			std::string auth=to_hex_string(hash_sha256(request["params"]["auth"].asString()));
			request["params"]["auth"]=to_hex_string(hmac_sha256(auth,path+opts));
		}
		batch.append(request);
	}

	std::vector<starequest_t> old_queue=queue;
	queue.clear();

	try
	{
		//Make the request.
		std::string data=JSON_serialize(batch);
		std::string server_response=wget(superstar+"/superstar/",data);

		//Parse response, call responses.
		Json::Value response=JSON_deserialize(server_response);

		//Got an array, must be batch data...
		if(response.isArray())
		{
			for(Json::ArrayIndex ii=0;ii<response.size();++ii)
			{
				if(!response[ii].isMember("id"))
					continue;

				//Error callback...
				if(response[Json::ArrayIndex(ii)].isMember("error"))
				{
					Json::Value response_obj=response[Json::ArrayIndex(ii)]["error"];
					handle_error(old_queue[response[Json::ArrayIndex(ii)]["id"].asUInt()].error_cb,response_obj);
					continue;
				}

				//Success callback...
				if(response[Json::ArrayIndex(ii)].isMember("result")&&
					old_queue[response[Json::ArrayIndex(ii)]["id"].asUInt()].success_cb!=NULL)
				{
					Json::Value response_obj=response[Json::ArrayIndex(ii)]["result"];
					old_queue[response[Json::ArrayIndex(ii)]["id"].asUInt()].success_cb(response_obj);
				}
			}
		}

		//Server error...
		else
			for(size_t ii=0;ii<old_queue.size();++ii)
				handle_error(old_queue[ii].error_cb,response["error"]);
	}
	catch(std::exception& error)
	{
		Json::Value error_obj;
		error_obj["code"]=0;
		error_obj["message"]=error.what();
		for(size_t ii=0;ii<old_queue.size();++ii)
			handle_error(old_queue[ii].error_cb,error_obj);
	}
	catch(...)
	{
		Json::Value error_obj;
		error_obj["code"]=0;
		error_obj["message"]="Unknown error.";
		for(size_t ii=0;ii<old_queue.size();++ii)
			handle_error(old_queue[ii].error_cb,error_obj);
	}
}

//Function to handle errors...
void superstar_t::handle_error(error_cb_t error_cb,const Json::Value& error)
{
	if(error_cb!=NULL)
		error_cb(error);
	else
		std::cout<<"Superstar error ("<<error["code"].asString()<<") - "<<error["message"].asString()<<std::endl;
}