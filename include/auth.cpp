#include "auth.hpp"

#include "picosha2.h"
#include <vector>

static const size_t SHA256_HASH_SIZE=32;
static const size_t SHA256_BLOCK_SIZE=64;

//Performs SHA256 hash on given plain text, outputs a string of bytes.
std::string hash_sha256(const std::string& plain)
{
	std::vector<unsigned char> hash(SHA256_HASH_SIZE,'\0');
	picosha2::hash256(plain,hash);
	return std::string((char*)&(hash[0]),SHA256_HASH_SIZE);
}

//Computes HMACSHA256 on given plain text and key, outputs a string of bytes.
std::string hmac_sha256(std::string key,const std::string& plain)
{
	if(key.size()>SHA256_BLOCK_SIZE)
		key=hash_sha256(key);

	std::string o_key_pad(SHA256_BLOCK_SIZE,0x5c);
	std::string i_key_pad(SHA256_BLOCK_SIZE,0x36);

	for(size_t ii=0;ii<key.size();++ii)
	{
		o_key_pad[ii]^=key[ii];
		i_key_pad[ii]^=key[ii];
	}

	std::string hash=hash_sha256(i_key_pad+plain);
	return hash_sha256(o_key_pad+hash);
}