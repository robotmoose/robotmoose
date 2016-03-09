#ifndef URI_C11_HPP
#define URI_C11_HPP

#include <map>
#include <mongoose/mongoose.h>
#include <string>

std::map<std::string,std::string> uri_parse_query(const std::string& uri);

bool send_jpg(const mg_connection& connection,const std::string& jpg);
std::string get_jpg(const std::string& host,const std::string& name);

#endif