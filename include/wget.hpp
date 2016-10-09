#ifndef WGET_HPP_INCLUDED
#define WGET_HPP_INCLUDED

#include "mongoose/mongoose.h"
#include <string>

std::string wget(const std::string& address,const std::string& post_data);

#endif // WGET_HPP_INCLUDED