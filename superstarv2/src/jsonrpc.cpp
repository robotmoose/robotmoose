//Mike Moss
//07/09/2016
//Contains an implementation of the jsonrpc 2.0 specification: http://www.jsonrpc.org/specification

#include "jsonrpc.hpp"

#include "json_util.hpp"

//Attempts to handle and return the response of the jsonrpc request(s) object in
//  post_data via the passed superstar object.
std::string jsonrpc(superstar_t& superstar,const std::string& post_data)
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
				responses.append(jsonrpc_handle(superstar,requests[(Json::ArrayIndex)ii]));
		}

		//Single request...
		else
		{
			responses=jsonrpc_handle(superstar,requests);
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
	return JSON_serialize(responses);
}

//Handles individual jsonrpc request via the passed superstar object.
Json::Value jsonrpc_handle(superstar_t& superstar,const Json::Value request)
{
	Json::Value response=jsonrpc_skeleton();

	//Invalid request object.
	if(!request.isObject()||request["jsonrpc"]!="2.0"
		||jsonrpc_invalid_id(request)||jsonrpc_invalid_method(request))
	{
		response["error"]=jsonrpc_error(-32600,"Invalid Request");
	}

	//Supported methods.
	else if(jsonrpc_supported_method(request))
	{
		Json::Value params=request["params"];
		Json::Value method=request["method"];

		if(params.isObject()&&method=="get"&&params["opts"].isObject())
		{
			response["result"]=superstar.get(params["opts"]["path"].asString());
		}
		else if(params.isObject()&&method=="set"&&params["opts"].isObject())
		{
			response["result"]=true;
			superstar.set(params["opts"]["path"].asString(),params["opts"]["value"],
				params["auth"]);
		}
		else if(params.isObject()&&method=="sub"&&params["opts"].isObject())
		{
			response["result"]=superstar.sub(params["opts"]["path"].asString());
		}
		else if(params.isObject()&&method=="push"&&params["opts"].isObject()&&
			(params["opts"]["length"].isUInt()||params["opts"]["length"].isNull()))
		{
			response["result"]=true;
			superstar.push(params["opts"]["path"].asString(),params["opts"]["value"],
				params["opts"]["length"].asUInt(),params["auth"]);
		}
		else
		{
			response["error"]=jsonrpc_error(-32602,"Invalid params");
		}
	}

	//Unsupported methods.
	else
	{
		response["error"]=jsonrpc_error(-32601,"Method not found");
	}

	//Set id if it was passed (as per spec).
	if(request.isObject()&&!jsonrpc_invalid_id(request))
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
bool jsonrpc_invalid_id(const Json::Value request)
{
	return (request["id"].isBool()||
		request["id"].isArray()||
		request["id"].isObject());
}

//Checkes if the given request's method is a valid one (as in the specification).
//  Note, same mentality here as in the invalid id function, need a string.
bool jsonrpc_invalid_method(const Json::Value request)
{
	return (request["method"].isNull()||
			request["method"].isBool()||
			request["method"].isInt()||
			request["method"].isUInt()||
			request["method"].isIntegral()||
			request["method"].isDouble()||
			request["method"].isNumeric()||
			request["method"].isArray()||
			request["method"].isObject());
}

//Checkes if the given request's method is supported (as in a get/set/sub/push).
bool jsonrpc_supported_method(const Json::Value request)
{
	return (request["method"]=="get"||
		request["method"]=="set"||
		request["method"]=="sub"||
		request["method"]=="push");
}