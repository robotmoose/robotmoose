#include "cppcheck.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>

#if(defined(_WIN32)&&!defined(__CYGWIN__))

	static FILE* popen(const char* command,const char* type)
	{
		return _popen(command,type);
	}

	static int pclose(FILE* stream);
	{
		return _pclose(stream);
	}

#endif

static bool exec(const std::string& cmd,std::string& out)
{
	std::string temp="";
	FILE* pipe=popen(cmd.c_str(),"r");

	if(pipe==nullptr)
		return false;

	char buffer[128];

	while(!feof(pipe))
		if(fgets(buffer,128,pipe)!=nullptr)
			temp+=buffer;

	pclose(pipe);

	out=temp;
	return true;
}

static std::vector<std::string> data_to_lines(const std::string& data)
{
	std::vector<std::string> lines;
	std::istringstream istr(data);
	std::string temp;

	while(std::getline(istr,temp))
		lines.push_back(temp);

	return lines;
}

static int parse_int(const std::string& str)
{
	std::istringstream istr(str);
	int ret=-1;
	istr>>ret;
	return ret;
}

inline bool file_exists(const std::string& name)
{
	FILE* file=fopen(name.c_str(),"r");

	if(file!=nullptr)
	{
		fclose(file);
		return true;
	}

	return false;
}

static bool parse_errors(const std::string& file_name,const std::string& error_data,std::vector<myerror_t>& errors)
{
	auto lines=data_to_lines(error_data);
	std::vector<myerror_t> temp_errors;

	for(size_t ii=0;ii<lines.size();++ii)
	{
		auto file_name_end=lines[ii].find(file_name+":");

		if(file_name_end==std::string::npos)
		{
			lines.erase(lines.begin()+ii);
			--ii;
			continue;
		}

		file_name_end+=file_name.size()+1;

		lines[ii]=lines[ii].substr(file_name_end,lines[ii].size()-file_name_end);

		int line_number=parse_int(lines[ii]);

		if(line_number==-1)
			continue;

		for(auto error:temp_errors)
			if(error.line==(size_t)line_number)
				continue;

		std::vector<std::string> error_texts={": error: ",": warning: ",": "};

		size_t error_index=std::string::npos;
		std::string error_text;

		for(auto possible_error_text:error_texts)
		{
			error_index=lines[ii].find(possible_error_text);

			if(error_index!=std::string::npos)
			{
				error_text=possible_error_text;
				break;
			}
		}

		if(error_index==std::string::npos)
		{
			lines.erase(lines.begin()+ii);
			--ii;
			continue;
		}

		error_index+=error_text.size();

		lines[ii]=lines[ii].substr(error_index,lines[ii].size()-error_index);
		temp_errors.push_back({(size_t)line_number,0,lines[ii]});
	}

	errors=temp_errors;
	return true;
}

bool cppcheck(const size_t identifier,const std::string& file_data,std::vector<myerror_t>& errors)
{
	std::string identifier_str=std::to_string(identifier);
	std::string file_name=identifier_str+".cpp";
	std::string temp;

	if(!exec("mkdir -p "+identifier_str,temp))
		return false;

	std::ofstream ostr(identifier_str+"/"+file_name);

	if(ostr<<file_data)
	{
		ostr.close();

		std::string error_data;
		std::string remove_data;

		auto command1="cd \""+identifier_str+"\"&&g++ -Wall -g \""+file_name+"\" 2>&1|grep \""+file_name+":\"";
		auto command2="rm -rf \""+identifier_str+"\"";

		auto ret=exec(command1,error_data)&&exec(command2,remove_data);

		if(!ret)
			return false;

		return parse_errors(file_name,error_data,errors);
	}

	return false;
}

bool cppcheck_anonymous(const std::string& file_data,std::vector<myerror_t>& errors,const size_t tries)
{
	for(size_t ii=0;ii<tries;++ii)
		if(cppcheck(rand(),file_data,errors))
			return true;
	return false;
}