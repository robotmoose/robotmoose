#include <ini.h>
#include <iostream>

int main()
{
	ini_t ini=read_ini("test.ini");

	for(ini_t::const_iterator iter=ini.begin();iter!=ini.end();++iter)
		std::cout<<"\""<<iter->first<<"\":\""<<iter->second<<"\","<<std::endl;

	return 0;
}