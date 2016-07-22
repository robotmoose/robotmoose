//Mike Moss
//07/21/2016
//Contains helper functions for mongoose.

#include "mongoose_util.hpp"

//Helper to send a message to conn with given status and content.
// Note, status should be in the form "200 OK" or "401 Unauthorized".
void mg_send(mg_connection* conn,const std::string& status,const std::string& content)
{
	mg_printf(conn,
		"HTTP/1.1 %s\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: %ld\r\n"
		"\r\n"
		"%s",
		status.c_str(),content.size(),content.c_str());
}