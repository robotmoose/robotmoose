#ifndef WGET_HPP_INCLUDED
#define WGET_HPP_INCLUDED

#include "mongoose/mongoose.h"
#include <string>

void wget_ev_handler(mg_connection* connection,int ev,void* ev_data);
std::string wget(const std::string& address,const std::string& post_data);

#endif // WGET_HPP_INCLUDED