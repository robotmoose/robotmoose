#ifndef CPP_CHECK_HPP_C11
#define CPP_CHECK_HPP_C11

#include <string>
#include <vector>

struct myerror_t
{
	ssize_t line;
	size_t column;
	std::string text;
};

bool cppcheck(const size_t identifier,const std::string& file_data,std::vector<myerror_t>& errors);
bool cppcheck_arduino(const size_t identifier,const std::string& file_data,std::vector<myerror_t>& errors);
bool cppcheck_anonymous(const std::string& file_data,std::vector<myerror_t>& errors,const size_t tries=1);
bool cppcheck_arduino_anonymous(const std::string& file_data,std::vector<myerror_t>& errors,const size_t tries=1);

#endif