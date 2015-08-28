//Windows Dependencies:
//		-lcrypto -lgdi32 -lpthread

//Linux Dependencies:
//		-lcrypto -lpthread

#ifndef MSL_C11_CRYPTO_HPP
#define MSL_C11_CRYPTO_HPP

#include <stdexcept>
#include <string>

namespace msl
{
	class encryption_error:public std::runtime_error
	{
		public:
			encryption_error(const std::string& str);
	};

	class decryption_error:public std::runtime_error
	{
		public:
			decryption_error(const std::string& str);
	};

	class hash_error:public std::runtime_error
	{
		public:
			hash_error(const std::string& str);
	};

	std::string encrypt_rsa(const std::string& plain,const std::string& key);
	std::string decrypt_rsa(const std::string& cipher,const std::string& key);

	std::string encrypt_aes256(const std::string& plain,const std::string& key,const std::string& iv);
	std::string decrypt_aes256(const std::string& cipher,const std::string& key,const std::string& iv);

	std::string hash_md5(const std::string& plain);
	std::string hash_sha160(const std::string& plain);
	std::string hash_sha256(const std::string& plain);
	std::string hash_sha512(const std::string& plain);

	std::string hmac_md5(std::string key,const std::string& plain);
	std::string hmac_sha160(std::string key,const std::string& plain);
	std::string hmac_sha256(std::string key,const std::string& plain);
	std::string hmac_sha512(std::string key,const std::string& plain);

	std::string pbkdf2(const std::string& password,const std::string& salt,
		const size_t key_byte_size,const size_t iterations);
}

#endif