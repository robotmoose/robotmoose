#include "cppcheck.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>

//Requires linux system with avr-g++ and avr-objcopy in bin search path.

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
		auto undefined_start=lines[ii].find("undefined reference to");

		if(undefined_start!=std::string::npos)
		{
			auto str=lines[ii].substr(undefined_start,lines[ii].size()-undefined_start);
			temp_errors.push_back({-1,0,str});
			continue;
		}

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
			if(error.line==(ssize_t)line_number)
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
		temp_errors.push_back({(ssize_t)line_number,0,lines[ii]});
	}

	for(size_t ii=1;ii<temp_errors.size();++ii)
	{
		if(temp_errors[ii-1].line==temp_errors[ii].line&&
			temp_errors[ii-1].column==temp_errors[ii].column&&
			temp_errors[ii-1].text==temp_errors[ii].text)
		{
			temp_errors.erase(temp_errors.begin()+ii);
			--ii;
		}
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

		std::string compile_command="avr-g++ -I../arduino/hardware/variants/standard -mmcu=atmega328p -DF_CPU=16000000UL -Iarduino/hardware/cores -I../arduino/hardware/cores/arduino ../arduino/hardware/cores/arduino/*.cpp ../arduino/hardware/cores/arduino/*.c -o test.elf -Wno-sign-compare -Wno-unused-variable -Wall -lm -Wl,--gc-sections -DUSB_VID=null -DUSB_PID=null -DARDUINO=103 -Wno-strict-aliasing";
		std::string link_command="avr-objcopy -R .eeprom -O ihex test.elf test.hex";

		auto command1="cd \""+identifier_str+"\"&&"+compile_command+" \""+file_name+"\" 2>&1 && "+link_command+" 2>&1|grep \""+file_name+":\"";
		auto command2="rm -rf \""+identifier_str+"\"";

		auto ret=exec(command1,error_data)&&exec(command2,remove_data);

		if(!ret)
			return false;

		return parse_errors(file_name,error_data,errors);
	}

	return false;
}

bool cppcheck_arduino(const size_t identifier,const std::string& file_data,std::vector<myerror_t>& errors)
{
	if(cppcheck(identifier,"#include<Arduino.h>\n"+file_data,errors))
	{
		for(auto& error:errors)
			--error.line;

		return true;
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

bool cppcheck_arduino_anonymous(const std::string& file_data,std::vector<myerror_t>& errors,const size_t tries)
{
	for(size_t ii=0;ii<tries;++ii)
		if(cppcheck_arduino(rand(),file_data,errors))
			return true;
	return false;
}