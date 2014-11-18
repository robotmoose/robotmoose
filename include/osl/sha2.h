/*
SHA-256 Hash in C++

2013-02-19 : Orion Lawlor : Public domain
2010-06-11 : Igor Pavlov : Public domain
http://cpansearch.perl.org/src/BJOERN/Compress-Deflate7-1.0/7zip/C/Sha256.c
This code is based on public domain code from Wei Dai's Crypto++ library.
*/
#ifndef __OSL_SHA256_H
#define __OSL_SHA256_H

#include <string.h> /* for size_t and memset (to zero) */
#include <string> /* for std::string */

/*
  This class computes SHA256 message digests.
*/
class SHA256 {
public:
	/* This type needs to be at least 32 bits, unsigned */
	typedef unsigned int UInt32; 
	/* This is the type of the data you're processing */
	typedef unsigned char Byte;
	
	/* This is the data type of a message digest. */
	class digest {
	public:
		enum {size=32}; // bytes in a message digest
		SHA256::Byte data[size]; // binary digest data
		
		// Equality.  This is useful for "if (cur==target)" tests.
		bool operator==(const digest &other) const {
			for (int i=0;i<size;i++)
				if (data[i]!=other.data[i])
					return false;
			return true;
		}
		
		// Less-than.  This is mostly useful for std::map<SHA256::digest, ...>
		bool operator<(const digest &other) const {
			for (int i=0;i<size;i++)
				if (data[i]<other.data[i])
					return true;
				else if (data[i]>other.data[i])
					return false;
			return false;
		}
		
		// Convert digest to an ASCII string of hex digits (for printouts)
		std::string toHex() const;
	};
	
/* External Interface */
	SHA256(); // constructor.  Sets up initial state.

	// Add raw binary message data to our hash. 
	//  You can call this repeatedly to add as much data as you want.
	void add(const void *data, size_t size);
	
	// Finish this message and extract the digest. 
	// Resets so you can add the next message, if desired.
	SHA256::digest finish(void);
	
	~SHA256(); // destructor.  Clears out state and buffered data.
	
/* Internal Interface (public, for debug's sake) */
	// This is the internal state of the hash.
	UInt32 state[8];
	
	// This is how many message bytes we've seen so far.
	size_t count;
	
	// This buffers up to a whole block of data
	Byte buffer[64];
	
	
	// Reset to initial values.
	void init();
	
	// Process the finished block of data in "buffer"
	void block();
};


/* This is the *really* easy version: given a string as input, return the digest as output. 
     std::cout<<"SHA-256: "<<SHA256_digest(someString).toHex()<<"\n";
*/
inline SHA256::digest SHA256_digest(const std::string &src) {
	SHA256 hash;
	hash.add(&src[0],src.length());
	return hash.finish();
}

#endif

