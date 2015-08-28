#ifndef MSL_C11_XMLHTTP_HPP
#define MSL_C11_XMLHTTP_HPP

#include <string>
#include <utility>
#include <vector>

namespace msl
{
	struct http_response
	{
		std::string proto;
		std::vector<std::pair<std::string,std::string>> pairs;
		std::string data;
	};

	http_response parse_http_response(std::string response);
	http_response get_request(const std::string& host,const std::string& name);
	http_response post_request(const std::string& host,const std::string& name,const std::string& data);
}

#endif