#ifndef HTTP_UTIL_HPP
#define HTTP_UTIL_HPP

#include <mongoose/mongoose.h>
#include <stdlib.h>
#include <string>

inline int send_json(mg_connection* connection,const std::string& json)
{
	mg_printf(connection,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/json\r\n"
		"Content-Length: %ld\r\n"
		"\r\n"
		"%s",
		json.size(), json.c_str());

	return MG_TRUE;
}

std::string get_var(mg_connection* connection,const std::string& key)
{
	const size_t nbuf=32767;
	char buf[nbuf];
	int size=mg_get_var(connection,key.c_str(),buf,nbuf);

	if(size<0)
		return "";

	return std::string(buf,size);
}

bool open_site(const std::string& site)
{
	#ifdef _WIN32
		return system(("start "+site).c_str())==0;
	#elif __APPLE__
		return system(("open "+site).c_str())!=-1;
	#elif __linux__
		return system(("xdg-open "+site).c_str())!=-1;
	#else
		return false;
	#endif
}

#endif