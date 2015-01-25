#include "uri.hpp"

#include <msl/socket.hpp>
#include <uriparser/Uri.h>

std::map<std::string,std::string> uri_parse_query(const std::string& query)
{
	UriParserStateA state;
	UriUriA uri;
	state.uri=&uri;
	UriQueryListA* list;

	if(uriParseUriA(&state,query.c_str())!=0||
		uriDissectQueryMallocA(&list,nullptr,uri.query.first,uri.query.afterLast)!=0)
	{
		uriFreeUriMembersA(&uri);
		return {{"error","Bad string."}};
	}

	std::map<std::string,std::string> queries;
	auto head=list;

	while(head!=nullptr)
	{
		if(head->key!=nullptr)
		{
			if(head->value!=nullptr)
				queries[std::string(head->key)]=std::string(head->value);
			else
				queries[std::string(head->key)]="true";
		}
		head=head->next;
	}

	uriFreeQueryListA(list);
	uriFreeUriMembersA(&uri);
	return queries;
}

bool send_jpg(const mg_connection& connection,const std::string& jpg)
{
	return mg_send_data((mg_connection*)&connection,jpg.data(),jpg.size())>=0;
}

std::string get_jpg(const std::string& host,const std::string& name)
{
	try
	{
		msl::tcp_socket_t get("0.0.0.0:0>"+host);
		get.open();

		if(!get.good())
			return "";

		std::string request="GET /"+name+" HTTP/1.1\r\n";
		request+="Connection: close\r\n";
		request+="\r\n";
		get.write(request);

		std::string jpg="";
		uint8_t temp;

		while(get.available()>=0&&get.read(&temp,1)==1)
			jpg+=temp;

		get.close();

		bool found=false;

		for(size_t ii=0;ii<jpg.size();++ii)
		{
			if(ii+3<jpg.size()&&jpg[ii]==(char)0xff&&jpg[ii+1]==(char)0xd8&&jpg[ii+2]==(char)0xff&&jpg[ii+3]==(char)0xe0)
			{
				found=true;
				jpg=jpg.substr(ii,jpg.size()-ii);
				break;
			}
		}

		if(!found)
			return "";

		return jpg;
	}
	catch(...)
	{}

	return "";
}