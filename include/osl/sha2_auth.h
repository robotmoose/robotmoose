#ifndef __OSL_SHA2_AUTH_H
#include "sha2.h"

/** Get the authentication code for this data. */
template <typename SHA>
inline std::string getAuthCode(const std::string &alldata) {
	SHA h;
	h.add(&alldata[0],alldata.size());
	typename SHA::digest d=h.finish();
	
	// Auth code character set, alphanumeric but skipping similar letters "1IlO0"
	const char table[]="ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
	const unsigned int table_length=sizeof(table)-1;
	unsigned int lastcopy=((1<<16)/table_length)*table_length; // 16 bit chunks
	
	// Make auth code by folding 16-bit chunks of the hash together.
	//   The resulting collisions mean lots of hashes fit the same password.
	std::string auth="";
	for (unsigned int i=0;i+2<=SHA::digest::size;i+=2) {
		unsigned int n=(d.data[i]<<8)|(d.data[i+1]);
		if (n<lastcopy) auth += table[n%table_length];
		// else skip this part of the key (avoids bias toward low letters)
	}
	return auth;
}

#endif

