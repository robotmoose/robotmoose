#include "superstar2_c.h"
#include <iostream>

int main(){
    superstar_session_t sst("http://localhost:8081/superstar");
    sst.set_key("");
    auto r = sst.get("/");
    std::cout<<r<<std::endl;
    r = sst.set("/", 4);
    std::cout<<r<<std::endl;
    r = sst.sub("/");
    std::cout<<r<<std::endl;
    Json::Value t;
    t[0] = "a";
    t[1] = "b";
    r = sst.push("/", t);
    std::cout<<r<<std::endl;

    return 0;
}
