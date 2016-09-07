#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string_util.hpp>
#include <superstar.hpp>
#include <time_util.hpp>

typedef std::function<void(size_t,const std::string&,const std::string&,const std::string&)> test_func_t;

void get(size_t iters,const std::string& hostname,const std::string& path,const std::string& ignore)
{
	superstar_t ss(hostname);
	for(size_t ii=0;ii<iters;++ii)
	{
		ss.get(path);
		ss.flush();
	}
}


void set(size_t iters,const std::string& hostname,const std::string& path,const std::string& value)
{
	superstar_t ss(hostname);
	for(size_t ii=0;ii<iters;++ii)
	{
		ss.set(path,value,"");
		ss.flush();
	}
}

void do_test(std::string test_name,test_func_t test,size_t iters,const std::string& hostname,const std::string& path,const std::string& value)
{
	std::cout<<"Test:               "<<test_name<<std::endl;
	auto t0=std::chrono::high_resolution_clock::now();
	test(iters,hostname,path,value);
	auto t1=std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> delta_t=t1-t0;
	std::cout<<"Total Time (s):     "<<delta_t.count()<<std::endl;
	std::cout<<"Time Per Call (ms): "<<delta_t.count()/(double)iters*1000<<std::endl;
	std::cout<<std::endl;
}

int main(int argc,char* argv[])
{
	std::cout<<"USAGE: "<<argv[0]<<" [ITERATIONS] [HOSTNAME] [SET_PATH]"<<std::endl;
	std::cout<<"  NOTE: THIS DOES NOT TEST LATENCY FOR DIFFERENT SIZED PACKETS (AS IT SHOULD)."<<std::endl;
	std::cout<<std::endl;

	size_t ITERS=1000;
	std::string HOSTNAME="http://test.robotmoose.com/superstar";
	std::string PATH="/robots/2016/auto/gen/benchmark";

	if(argc>1)
		ITERS=std::stoi(argv[1]);
	if(argc>2)
		HOSTNAME=argv[2];
	if(argc>3)
		PATH=argv[3];

	std::cout<<"Iterations: "<<ITERS<<std::endl;
	std::cout<<"Hostname:   "<<HOSTNAME<<std::endl;
	std::cout<<"Set Path:   "<<PATH<<std::endl;
	std::cout<<std::endl;

	do_test("get root",get,ITERS,HOSTNAME,"/","");
	do_test("set specific value",set,ITERS,HOSTNAME,PATH,R"({"poem":"The berries are nice today"})");

	return 0;
}
