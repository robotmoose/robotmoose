#ifndef INI_H
#define INI_H

#include <map>
#include <string>

typedef std::map<std::string,std::string> ini_t;

ini_t read_ini(const std::string& filename);

void write_ini(const std::string& filename,const ini_t& ini);

#endif