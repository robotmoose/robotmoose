//Mike Moss
//07/09/2016
//Contains string based helper functions (inspired by python).

#ifndef STRING_UTIL_HPP
#define STRING_UTIL_HPP

#include <string>
#include <vector>

//Returns whether str starts with prefix.
bool starts_with(const std::string& str,const std::string& prefix);

//Returns whether str ends with suffix.
bool ends_with(const std::string& str,const std::string& suffix);

//Strips contiguous blocks of match from start of str.
std::string strip_start(std::string str,const std::string& match);

//Strips contiguous blocks of match from end of str.
std::string strip_end(std::string str,const std::string& match);

//Strips contiguous blocks of match from start and end of str.
std::string strip(std::string str,const std::string& match);

//Replace all instances of find in str with replace.
std::string replace_all(std::string str,const std::string& find,const std::string& replace);

//Splits str into tokens separated by delim.
//  If str is "", then no tokens are returned.
//  If delim is "" and str isn't "" then one token is returned (str).
//  Trailing and ending tokens add in empty tokens:
//    So str==",1,2," and delim=="," would yield ["","1","2",""].
std::vector<std::string> split(const std::string& str,const std::string& delim);

//Converts bytes of str into a hex string.
std::string to_hex_string(const std::string& str);

#endif