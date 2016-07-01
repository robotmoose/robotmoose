#ifndef MSL_C11_FILE_HPP
#define MSL_C11_FILE_HPP

#include <string>
#include <vector>

namespace msl
{
	std::vector<std::string> list_directories(const std::string& path);
	std::vector<std::string> list_files(const std::string& path);

	bool file_to_string(const std::string& filename,std::string& data);
	bool string_to_file(const std::string& data,const std::string& filename);
};

#endif