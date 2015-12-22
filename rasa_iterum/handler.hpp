#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "backend.hpp"
#include "http_util.hpp"
#include <iostream>
#include <mongoose/mongoose.h>
#include <stdexcept>
#include <string>
#include "string_util.hpp"

inline int http_handler(struct mg_connection* connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(event!=MG_REQUEST)
		return false;

	backend_t* backend=(backend_t*)connection->server_param;
	std::string method=connection->request_method;
	std::string uri=std::string(connection->uri);

	std::string query;
	if(connection->query_string!=NULL)
		query=std::string(connection->query_string);

	std::string post_data;
	if(connection->content!=NULL)
		post_data=std::string(connection->content,connection->content_len);

	//DEBUG
	std::cout<<"METHOD:      "<<method<<std::endl;
	std::cout<<"URI:         "<<uri<<std::endl;
	std::cout<<"Query:       "<<query<<std::endl;
	std::cout<<"Post Bytes:  "<<post_data.size()<<std::endl;

	std::string return_json="{";

	if(get_var(connection,"port").size()>0)
	{
		return_json+="\"port\":\""+backend->port()+"\"";
	}

	if(get_var(connection,"ports").size()>0)
	{
		if(return_json.size()>1)
			return_json+=",";

			return_json+="\"ports\":"+to_json_array(backend->ports());
	}

	if(get_var(connection,"robot").size()>0)
	{
		if(return_json.size()>1)
			return_json+=",";

		return_json+="\"robot\":\""+backend->robot()+"\"";
	}

	if(get_var(connection,"school").size()>0)
	{
		if(return_json.size()>1)
			return_json+=",";

		return_json+="\"school\":\""+backend->school()+"\"";
	}

	if(get_var(connection,"set").size()>0)
	{
		if(return_json.size()>1)
			return_json+=",";

		return_json+="\"set\":\""+backend->set(post_data)+"\"";
	}

	if(get_var(connection,"status").size()>0)
	{
		if(return_json.size()>1)
			return_json+=",";

		return_json+="\"status\":\""+backend->status()+"\"";
	}

	if(return_json.size()>1)
		return send_json(connection,return_json+"}");

	return MG_FALSE;
}

#endif