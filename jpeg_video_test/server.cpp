#include <iostream>
#include <msl/time.hpp>
#include <msl/webserver.hpp>
#include "uri.hpp"
#include <stdexcept>

bool client_func(const mg_connection& connection,enum mg_event event);

msl::webserver_t server(client_func,"0.0.0.0:80","web");

int main()
{
	server.open();

	if(server.good())
		std::cout<<":)"<<std::endl;
	else
		std::cout<<":("<<std::endl;

	while(server.good())
		msl::delay_ms(1);

	server.close();

	return 0;
}

bool client_func(const mg_connection& connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(event!=MG_REQUEST)
		return false;

	if(connection.uri!=nullptr)
	{
		std::string request=connection.uri;
		std::cout<<connection.remote_ip<<":"<<connection.remote_port<<"\t"<<request<<std::endl;

		if(request=="/cam.jpg")
		{
			std::string who="";

			if(connection.query_string!=nullptr)
				for(auto ii:uri_parse_query("?"+std::string(connection.query_string)))
					if(ii.first=="who")
						who=ii.second;

			auto jpg=get_jpg(who,request);

			if(jpg!="")
			{
				send_jpg(connection,jpg);
				return true;
			}

			return false;
		}
	}

	return false;
}
