//Mike Moss
//07/21/2016
//Contains an implementation of the jsonrpc 2.0 specification:
//  http://www.jsonrpc.org/specification

#include "jsonrpc.hpp"

#include "json_util.hpp"
#include "mongoose_util.hpp"

//Attempts to handle and send the response of the jsonrpc request(s) object in
//  post_data via the passed superstar object.
//  Note, also handles comet connections.
void jsonrpc(superstar_t& superstar,comet_mgr_t& comet_mgr,
	const std::string& post_data,mg_connection* conn)
{
	//Might be one or a batch of responses.
	Json::Value responses;

	//Should be JSON throws only!!!
	try
	{
		Json::Value requests;
		requests=JSON_deserialize(post_data);

		//Batch
		if(requests.isArray())
		{
			//Batches of length zero aren't allowed...
			if(requests.size()==0)
			{
				responses=jsonrpc_skeleton();
				responses["error"]=jsonrpc_error(-32600,"Invalid Request");
			}

			//Handle each invidual request.
			for(size_t ii=0;ii<requests.size();++ii)
				responses.append(jsonrpc_handle(superstar,comet_mgr,
					requests[(Json::ArrayIndex)ii]));
		}

		//Single request...
		else
		{
			Json::Value response(jsonrpc_handle(superstar,comet_mgr,requests));

			//Comet requests...
			if(response.isMember("comet")&&response["comet"].asBool())
			{
				comet_mgr.handle(conn,response);
				return;
			}

			//Regular response...
			responses=response;
		}
	}

	//JSON parse error, add the actual error to the error object.
	catch(std::exception& error)
	{
		responses=jsonrpc_skeleton();
		responses["error"]=jsonrpc_error(-32700,"Parse error",error.what());
	}

	//Unknown JSON errors...
	catch(...)
	{
		responses=jsonrpc_skeleton();
		responses["error"]=jsonrpc_error(-32700,"Parse error");
	}

	//Either batch response or single response.
	mg_send(conn,"200 OK",JSON_serialize(responses));
}

//Handles individual jsonrpc request via the passed superstar/comet_mgr objects.
Json::Value jsonrpc_handle(superstar_t& superstar,comet_mgr_t& comet_mgr,
	const Json::Value request)
{
	Json::Value response=jsonrpc_skeleton();

	//Invalid request object.
	if(!request.isObject()||
		request["jsonrpc"].asString()!="2.0"||
		jsonrpc_invalid_id(request["id"])||
		JSON_isString(request["method"]))
	{
		response["error"]=jsonrpc_error(-32600,"Invalid Request");
	}

	//Supported methods.
	else if(jsonrpc_supported_method(request["method"]))
	{
		Json::Value params=request["params"];
		Json::Value method=request["method"];
		bool bad_params=false;
		std::string error_message="";
		std::string path;
		std::string opts_str;
		Json::Value opts;
		bool auth=true;

		//We only use objects as params...for now...
		if(!params.isObject())
		{
			bad_params=true;
			error_message="Params field must be an object.";
			goto error_label;
		}

		//We only use strings for paths...
		if(JSON_isString(params["path"]))
		{
			bad_params=true;
			error_message="Path field must be a string.";
			goto error_label;
		}
		path=params["path"].asString();

		//Try to decode opts string.
		try
		{
			//We only use strings for ENCODED opts...or null...
			//  Note, we aren't using the string helper function here...
			//  JSONCPP tries to be intelligent by treating string encoded objects
			//  as objects...so it doesn't work here...but .asString() works as expected...
			if(!params["opts"].isString()&&!params["opts"].isNull())
				return 0;

			//Only decode if not actually null (aka "")...
			opts_str=params["opts"].asString();
			if(opts_str.size()>0)
				opts=JSON_deserialize(opts_str);

			//We only use objects for DECODED opts...or null...
			if(!opts.isObject()&&!opts.isNull())
				return 0;
		}

		//Bad opts type or bad decode...
		catch(...)
		{
			bad_params=true;
			error_message="Opts field must be a string encoded json object or null.";
			goto error_label;
		}

		//We only use strings for auths...
		if(JSON_isString(params["auth"])&&!params["auth"].isNull())
		{
			bad_params=true;
			error_message="Auth field must be a string or null.";
			goto error_label;
		}

		//Write based operations - check auth.
		if((method=="set"||method=="push"||method=="change_auth")&&
			!superstar.auth_check(path,opts_str,params["auth"]))
		{
			auth=false;
			goto error_label;
		}

		//Otherwise service method.
		if(method=="get")
		{
			response["result"]=superstar.get(path);
		}
		else if(method=="set")
		{
			if(opts["value"]==Json::nullValue)
			{
				bad_params=true;
				goto error_label;
			}
			response["result"]=true;
			superstar.set(path,opts["value"]);
			comet_mgr.update_path(path,superstar);
		}
		else if(method=="sub")
		{
			response["result"]=superstar.sub(path);
		}
		else if(method=="push")
		{
			if(opts["value"]==Json::nullValue)
			{
				bad_params=true;
				goto error_label;
			}
			//Length is either null or an integer...optional field...
			if(!opts["length"].isUInt()&&!opts["length"].isNull())
			{
				bad_params=true;
				error_message="Length field must be an integer or null.";
				goto error_label;
			}

			response["result"]=true;
			superstar.push(path,opts["value"],opts["length"]);
			comet_mgr.update_path(path,superstar);
		}
		else if(method=="change_auth")
		{
			response["result"]=superstar.change_auth(path,opts["value"]);
		}
		else if(method=="get_next")
		{
			response["comet"]=true;
			response["path"]=path;
			response["result"]=superstar.get(path);
		}

		//What have I become?
		error_label:
			if(bad_params)
				response["error"]=jsonrpc_error(-32602,"Invalid params",error_message);
			if(!auth)
				response["error"]=jsonrpc_error(-32000,"Not authorized","");
	}

	//Unsupported methods.
	else
	{
		response["error"]=jsonrpc_error(-32601,"Method not found");
	}

	//Set id if it was passed (as per spec).
	if(request.isObject()&&!jsonrpc_invalid_id(request["id"]))
		response["id"]=request["id"];

	return response;
}

//Builds a jsonrpc error object with given code, message, and data (data is optional).
Json::Value jsonrpc_error(int code,const std::string& message,const std::string& data)
{
	Json::Value error;
	error["code"]=code;
	error["message"]=message;
	if(data.size()>0)
		error["data"]=data;
	return error;
}

//Build a skeleton jsonrpc response object (version and id).
Json::Value jsonrpc_skeleton()
{
	Json::Value skeleton;
	skeleton["jsonrpc"]="2.0";
	skeleton["id"]=Json::nullValue;
	return skeleton;
}

//Checkes if the given request's id is a valid one (as in the specification).
//  Note, id is supposed to be a string/number...JSONCPP seems to parse
//  everything as a string...so check for everything but a string to get
//  the desired effect.
bool jsonrpc_invalid_id(const Json::Value id)
{
	return (id.isBool()||
		id.isArray()||
		id.isObject());
}

//Checkes if the given request's method is supported (as in a get/set/sub/push/get_next).
bool jsonrpc_supported_method(const Json::Value method)
{
	std::string method_str=method.asString();
	return (method_str=="get"||
		method_str=="set"||
		method_str=="sub"||
		method_str=="push"||
		method_str=="change_auth"||
		method_str=="get_next");
}