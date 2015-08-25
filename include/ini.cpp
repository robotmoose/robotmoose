#include "ini.h"

#include <fstream>
#include <stdexcept>
#include <vector>

#include "string_util.h"

ini_t read_ini(const std::string& filename)
{
	ini_t ini;

	std::ifstream istr(filename.c_str());

	if(!istr)
		return ini;

	std::vector<std::string> lines;
	std::string temp;

	while(true)
	{
		if(!std::getline(istr,temp))
		{
			if(istr.eof())
				break;
			else
				throw std::runtime_error("Error while reading \""+filename+"\".");
		}
		else
		{
			lines.push_back(temp);
		}
	}

	istr.close();

	for(size_t ii=0;ii<lines.size();++ii)
	{
		if(lines.size()>0&&lines[ii][0]=='#')
			continue;

		std::string key;

		for(size_t jj=0;jj<lines[ii].size();++jj)
		{
			if(lines[ii][jj]=='=')
				break;
			else
				key+=lines[ii][jj];
		}

		std::string value;

		for(size_t jj=key.size()+1;jj<lines[ii].size();++jj)
			value+=lines[ii][jj];

		key=to_lower(strip_whitespace(key));
		value=to_lower(strip_whitespace(value));

		ini[key]=value;
	}

	return ini;
}

void write_ini(const std::string& filename,const ini_t& ini)
{
	std::ofstream ostr(filename.c_str());

	if(!ostr)
		throw std::runtime_error("Could not open \""+filename+"\" for writing.");

	std::vector<std::string> lines;

	for(ini_t::const_iterator iter=ini.begin();iter!=ini.end();++iter)
		lines.push_back(strip_whitespace(iter->first)+"="+strip_whitespace(iter->second));

	for(size_t ii=0;ii<lines.size();++ii)
		if(!(ostr<<lines[ii]<<std::endl))
			throw std::runtime_error("Error while writing \""+filename+"\".");

	ostr.close();
}