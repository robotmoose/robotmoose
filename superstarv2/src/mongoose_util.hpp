//Mike Moss
//07/21/2016
//Contains helper functions for mongoose.

#ifndef MONGOOSE_UTIL_HPP
#define MONGOOSE_UTIL_HPP

#include <mongoose/mongoose.h>
#include <string>

//Helper to send a message to conn with given status and content.
// Note, status should be in the form "200 OK" or "401 Unauthorized".
void mg_send(mg_connection* conn,const std::string& status,const std::string& content);

#endif