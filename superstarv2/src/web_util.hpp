#ifndef WEB_UTIL_HPP
#define WEB_UTIL_HPP

#include <string>

//URL encodes the given string.
//  As per: https://en.wikipedia.org/wiki/Percent-encoding
std::string url_encode(const std::string& str);

#endif