#include "file.hpp"

#include <fstream>

#if defined(_WIN32)&&!defined(__CYGWIN__)

#include <windows.h>

std::vector<std::string> msl::list_directories(const std::string& path)
{
	std::vector<std::string> files;
	const unsigned int buffer_size=65536;
	char buffer[buffer_size];

	if(GetFullPathName(path.c_str(),buffer_size,buffer,nullptr)==0)
		return files;

	std::string full_path_name(buffer);
	std::string full_path_wildcard=full_path_name+"\\*.*";
	WIN32_FIND_DATA file_descriptor;
	HANDLE file_handle=FindFirstFile(full_path_wildcard.c_str(),&file_descriptor);

	if(file_handle==INVALID_HANDLE_VALUE)
		return files;

	do
	{
		std::string node_name=(file_descriptor.cFileName);

		if(node_name!="."&&node_name!=".."&&(file_descriptor.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0)
			files.push_back(full_path_name+"\\"+node_name);
	}
	while(FindNextFile(file_handle,&file_descriptor));

	FindClose(file_handle);
	return files;
}

std::vector<std::string> msl::list_files(const std::string& path)
{
	std::vector<std::string> files;
	const unsigned int buffer_size=65536;
	char buffer[buffer_size];

	if(GetFullPathName(path.c_str(),buffer_size,buffer,nullptr)==0)
		return files;

	std::string full_path_name(buffer);
	std::string full_path_wildcard=full_path_name+"\\*.*";
	WIN32_FIND_DATA file_descriptor;
	HANDLE file_handle=FindFirstFile(full_path_wildcard.c_str(),&file_descriptor);

	if(file_handle==INVALID_HANDLE_VALUE)
		return files;

	do
	{
		std::string node_name=(file_descriptor.cFileName);

		if(node_name!="."&&node_name!=".."&&(file_descriptor.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0)
			files.push_back(full_path_name+"\\"+node_name);
	}
	while(FindNextFile(file_handle,&file_descriptor));

	FindClose(file_handle);
	return files;
}

#else

#include <dirent.h>
#include <stdlib.h>

std::vector<std::string> msl::list_directories(const std::string& path)
{
	std::vector<std::string> files;
	DIR* dp=opendir(path.c_str());
	while(dp!=nullptr)
	{
		dirent* np=readdir(dp);

		if(np==nullptr)
		{
			closedir(dp);
			break;
		}

		std::string node_name(np->d_name);

		if(node_name!="."&&node_name!=".."&&np->d_type==DT_DIR)
			files.push_back(node_name);
	}

	return files;
}

std::vector<std::string> msl::list_files(const std::string& path)
{
	std::vector<std::string> files;
	DIR* dp=opendir(path.c_str());

	while(dp!=nullptr)
	{
		dirent* np=readdir(dp);

		if(np==nullptr)
		{
			closedir(dp);
			break;
		}

		std::string node_name(np->d_name);

		if(node_name!="."&&node_name!=".."&&np->d_type!=DT_DIR)
			files.push_back(node_name);
	}

	return files;
}

#endif

bool msl::file_to_string(const std::string& filename,std::string& data)
{
	char buffer;
	std::ifstream istr(filename.c_str(),std::ios_base::in|std::ios_base::binary);
	istr.unsetf(std::ios_base::skipws);

	if(!istr)
		return false;

	data="";

	while(istr>>buffer)
		data+=buffer;

	istr.close();
	return true;
}

bool msl::string_to_file(const std::string& data,const std::string& filename)
{
	bool saved=false;
	std::ofstream ostr(filename.c_str(),std::ios_base::out|std::ios_base::binary);
	saved=(bool)(ostr<<data);
	ostr.close();
	return saved;
}