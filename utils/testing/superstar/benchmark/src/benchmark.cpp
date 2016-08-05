#include "superstar2_cpp.hpp"
#include <iostream>
using std::cout;
using std::endl;
#include <chrono>
#include <string>
using std::string;
#include <sstream>
#include <functional>
using std::function;

const string HOSTNAME = "http://localhost:8081/superstar";
const size_t ITERS = 1000;

void test_get_root(size_t iters, string hostname){
    superstar_session_t sst(hostname);
    for(size_t i=0;i<iters;++i){
        sst.get("/");
    }
}

void test_set_specific_key(size_t iters, string hostname){
    superstar_session_t sst(hostname);
    for(size_t i=0;i<iters;++i){
        sst.set("/foo/bar/", R"({"poem":"The berries are nice today"})");
    }
}

std::string print_results(std::chrono::duration<double> delta_t, size_t iters, string testname){
    std::ostringstream output;
    output<< "----> " << testname << ":\n";
    output<<"total time: "<<delta_t.count()<<" seconds\n";
    output<<"time per call: "<<delta_t.count()/(double)iters  * 1000<<" milliseconds\n";
    
    return output.str();
}

void do_test(string test_name, std::function<void (size_t, string)> test, size_t iters,  string hostname = HOSTNAME){
    auto t0 = std::chrono::high_resolution_clock::now();
    test(iters, hostname);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delta_t = t1-t0;
    cout<<print_results(delta_t, iters, test_name);
}

int main(){
    do_test("get root", test_get_root, ITERS);
    do_test("set specific value", test_set_specific_key, ITERS);

    return 0;
}
