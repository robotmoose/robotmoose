#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>

//Performs SHA256 hash on given plain text, outputs a string of bytes.
std::string hash_sha256(const std::string& plain);

//Computes HMACSHA256 on given plain text and key, outputs a string of bytes.
std::string hmac_sha256(std::string key,const std::string& plain);

#endif