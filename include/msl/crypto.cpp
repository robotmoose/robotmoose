//Windows Dependencies:
//		-lcrypto -lgdi32

//Linux Dependencies:
//		-lcrypto

#include "crypto.hpp"

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <cstdint>

#define RSA_PKCS1_PADDING_SIZE			11
#define RSA_PKCS1_OAEP_PADDING_SIZE		41

#define MD5_BLOCK_SIZE					64
#define SHA160_BLOCK_SIZE				64
#define SHA256_BLOCK_SIZE				64
#define SHA512_BLOCK_SIZE				128

#define AES_BLOCK_SIZE					16
#define AES256_KEY_SIZE					32

namespace msl
{
	class crypto_initialize_t
	{
		public:
			crypto_initialize_t()
			{
				ERR_load_crypto_strings();
				OpenSSL_add_all_algorithms();
			}

			crypto_initialize_t(const crypto_initialize_t& copy)=delete;

			~crypto_initialize_t()
			{
				ERR_free_strings();
				EVP_cleanup();
			}

			crypto_initialize_t& operator=(const crypto_initialize_t& copy)=delete;
	};
}

static msl::crypto_initialize_t crypto_initializer;

static void rsa_cleanup(BIO* keybio,RSA* rsa)
{
	BIO_free(keybio);
	RSA_free(rsa);
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
}

static void aes_cleanup(EVP_CIPHER_CTX* ctx)
{
	EVP_CIPHER_CTX_free(ctx);
	ERR_remove_state(0);
}

msl::encryption_error::encryption_error(const std::string& str):std::runtime_error(str)
{}

msl::decryption_error::decryption_error(const std::string& str):std::runtime_error(str)
{}

msl::hash_error::hash_error(const std::string& str):std::runtime_error(str)
{}

std::string msl::encrypt_rsa(const std::string& plain,const std::string& key)
{
	std::string cipher;
	BIO* keybio=nullptr;
	RSA* rsa=nullptr;

	try
	{
		keybio=BIO_new_mem_buf((uint8_t*)key.data(),-1);

		if(keybio==nullptr)
			throw msl::encryption_error("msl::encrypt_rsa() - BIO_new_mem_buf failed.");

		rsa=PEM_read_bio_RSA_PUBKEY(keybio,&rsa,nullptr,nullptr);

		if(rsa==nullptr)
			throw msl::encryption_error("msl::encrypt_rsa() - PEM_read_bio_RSA_PUBKEY failed.");

		if(plain.size()>(size_t)RSA_size(rsa)-RSA_PKCS1_OAEP_PADDING_SIZE)
			throw msl::encryption_error("msl::encrypt_rsa() - Invalid plain text size ("+
				std::to_string((size_t)RSA_size(rsa)-RSA_PKCS1_OAEP_PADDING_SIZE)+" max).");

		cipher.resize(RSA_size(rsa));

		size_t temp_length=RSA_public_encrypt(plain.size(),(uint8_t*)plain.data(),
			(uint8_t*)cipher.data(),rsa,RSA_PKCS1_OAEP_PADDING);

		if(temp_length==(size_t)~0)
			throw msl::encryption_error("msl::encrypt_rsa() - RSA_public_encrypt failed.");

		cipher.resize(temp_length);
	}
	catch(...)
	{
		rsa_cleanup(keybio,rsa);
		throw;
	}

	rsa_cleanup(keybio,rsa);
	return cipher;
}

std::string msl::decrypt_rsa(const std::string& cipher,const std::string& key)
{
	std::string plain;
	BIO* keybio=nullptr;
	RSA* rsa=nullptr;

	try
	{
		keybio=BIO_new_mem_buf((uint8_t*)key.data(),-1);

		if(keybio==nullptr)
			throw msl::decryption_error("msl::decrypt_rsa() - BIO_new_mem_buf failed.");

		rsa=PEM_read_bio_RSAPrivateKey(keybio,&rsa,nullptr,nullptr);

		if(rsa==nullptr)
			throw msl::decryption_error("msl::decrypt_rsa() - PEM_read_bio_RSAPrivateKey failed.");

		if(cipher.size()>(size_t)RSA_size(rsa))
			throw msl::decryption_error("msl::decrypt_rsa() - Invalid cipher text size ("+
				std::to_string((size_t)RSA_size(rsa))+" max).");

		plain.resize(RSA_size(rsa));

		size_t temp_length=RSA_private_decrypt(cipher.size(),(uint8_t*)cipher.data(),
			(uint8_t*)plain.data(),rsa,RSA_PKCS1_OAEP_PADDING);

		if(temp_length==(size_t)~0)
			throw msl::decryption_error("msl::decrypt_rsa() - RSA_private_decrypt failed.");

		plain.resize(temp_length);
	}
	catch(...)
	{
		rsa_cleanup(keybio,rsa);
		throw;
	}

	rsa_cleanup(keybio,rsa);
	return plain;
}

std::string msl::encrypt_aes256(const std::string& plain,const std::string& key,const std::string& iv)
{
	std::string cipher;
	cipher.resize((plain.size()/AES_BLOCK_SIZE+1)*AES_BLOCK_SIZE);
	EVP_CIPHER_CTX* ctx=nullptr;

	try
	{
		ctx=EVP_CIPHER_CTX_new();

		if(key.size()!=AES256_KEY_SIZE)
			throw msl::encryption_error("msl::encrypt_aes256() - Given key size is invalid ("+
				std::to_string(AES256_KEY_SIZE)+"bytes ).");

		int temp_length;
		int temp_unaligned_length;

		if(ctx==nullptr)
			throw msl::encryption_error("msl::encrypt_aes256() - Creating a EVP_CIPHER_CTX failed.");

		if(EVP_CIPHER_CTX_set_padding(ctx,1)==0)
			throw msl::encryption_error("msl::encrypt_aes256() - EVP_CIPHER_CTX_set_padding failed.");

		if(EVP_EncryptInit(ctx,EVP_aes_256_cbc(),(uint8_t*)key.data(),(uint8_t*)iv.data())==0)
			throw msl::encryption_error("msl::encrypt_aes256() - EVP_EncryptInit failed.");

		if(EVP_EncryptUpdate(ctx,(uint8_t*)cipher.data(),&temp_length,(uint8_t*)plain.data(),plain.size())==0)
			throw msl::encryption_error("msl::encrypt_aes256() - EVP_EncryptUpdate failed.");

		if(EVP_EncryptFinal(ctx,(uint8_t*)cipher.data()+temp_length,&temp_unaligned_length)==0)
			throw msl::encryption_error("msl::encrypt_aes256() - EVP_EncryptFinal failed.");


		cipher.resize(temp_length+temp_unaligned_length);
	}
	catch(...)
	{
		aes_cleanup(ctx);
		throw;
	}

	aes_cleanup(ctx);
	return cipher;
}

std::string msl::decrypt_aes256(const std::string& cipher,const std::string& key,const std::string& iv)
{
	std::string plain;
	plain.resize((cipher.size()/AES_BLOCK_SIZE+1)*AES_BLOCK_SIZE);
	EVP_CIPHER_CTX* ctx=nullptr;

	try
	{
		ctx=EVP_CIPHER_CTX_new();

		if(key.size()!=AES256_KEY_SIZE)
			throw msl::decryption_error("msl::decrypt_aes256() - Given key size is invalid ("+
				std::to_string(AES256_KEY_SIZE)+"bytes ).");

		int temp_length;
		int temp_unaligned_length;

		if(ctx==nullptr)
			throw msl::decryption_error("msl::decrypt_aes256() - Creating a EVP_CIPHER_CTX failed.");

		if(EVP_CIPHER_CTX_set_padding(ctx,1)==0)
			throw msl::decryption_error("msl::edecrypt_aes256() - EVP_CIPHER_CTX_set_padding failed.");

		if(EVP_DecryptInit(ctx,EVP_aes_256_cbc(),(uint8_t*)key.data(),(uint8_t*)iv.data())==0)
			throw msl::decryption_error("msl::encrypt_aes256() - EVP_DecryptInit failed.");

		if(EVP_DecryptUpdate(ctx,(uint8_t*)plain.data(),&temp_length,(uint8_t*)cipher.data(),cipher.size())==0)
			throw msl::decryption_error("msl::decrypt_aes256() - EVP_DecryptUpdate failed.");

		if(EVP_DecryptFinal(ctx,(uint8_t*)plain.data()+temp_length,&temp_unaligned_length)==0)
			throw msl::decryption_error("msl::decrypt_aes256() - EVP_DecryptFinal failed.");


		plain.resize(temp_length+temp_unaligned_length);
	}
	catch(...)
	{
		aes_cleanup(ctx);
		throw;
	}

	aes_cleanup(ctx);
	return plain;
}

std::string msl::hash_md5(const std::string& plain)
{
	MD5_CTX ctx;

	if(MD5_Init(&ctx)!=1)
		throw msl::hash_error("msl::hash_md5 - MD5_Init failed.");

	if(MD5_Update(&ctx,(unsigned char*)plain.data(),plain.size())!=1)
		throw msl::hash_error("msl::hash_md5 - MD5_Update failed.");

	std::string hash;
	hash.resize(MD5_DIGEST_LENGTH);

	if(MD5_Final((unsigned char*)hash.data(),&ctx)!=1)
		throw msl::hash_error("msl::hash_md5 - MD5_Final failed.");

	return hash;
}

std::string msl::hash_sha160(const std::string& plain)
{
	SHA_CTX ctx;

	if(SHA1_Init(&ctx)!=1)
		throw msl::hash_error("msl::hash_sha160 - SHA1_Init failed.");

	if(SHA1_Update(&ctx,(unsigned char*)plain.data(),plain.size())!=1)
		throw msl::hash_error("msl::hash_sha160 - SHA1_Update failed.");

	std::string hash;
	hash.resize(SHA_DIGEST_LENGTH);

	if(SHA1_Final((unsigned char*)hash.data(),&ctx)!=1)
		throw msl::hash_error("msl::hash_sha160 - SHA1_Final failed.");

	return hash;
}

std::string msl::hash_sha256(const std::string& plain)
{
	SHA256_CTX ctx;

	if(SHA256_Init(&ctx)!=1)
		throw msl::hash_error("msl::hash_sha256 - SHA256_Init failed.");

	if(SHA256_Update(&ctx,(unsigned char*)plain.data(),plain.size())!=1)
		throw msl::hash_error("msl::hash_sha256 - SHA256_Update failed.");

	std::string hash;
	hash.resize(SHA256_DIGEST_LENGTH);

	if(SHA256_Final((unsigned char*)hash.data(),&ctx)!=1)
		throw msl::hash_error("msl::hash_sha256 - SHA256_Final failed.");

	return hash;
}

std::string msl::hash_sha512(const std::string& plain)
{
	SHA512_CTX ctx;

	if(SHA512_Init(&ctx)!=1)
		throw msl::hash_error("msl::hash_sha512 - SHA512_Init failed.");

	if(SHA512_Update(&ctx,(unsigned char*)plain.data(),plain.size())!=1)
		throw msl::hash_error("msl::hash_sha512 - SHA512_Update failed.");

	std::string hash;
	hash.resize(SHA512_DIGEST_LENGTH);

	if(SHA512_Final((unsigned char*)hash.data(),&ctx)!=1)
		throw msl::hash_error("msl::hash_sha512 - SHA512_Final failed.");

	return hash;
}

std::string msl::hmac_md5(std::string key,const std::string& plain)
{
	if(key.size()>MD5_BLOCK_SIZE)
		key=msl::hash_md5(key);

	std::string o_key_pad(MD5_BLOCK_SIZE,0x5c);
	std::string i_key_pad(MD5_BLOCK_SIZE,0x36);

	for(size_t ii=0;ii<key.size();++ii)
	{
		o_key_pad[ii]^=key[ii];
		i_key_pad[ii]^=key[ii];
	}

	std::string hash=msl::hash_md5(i_key_pad+plain);
	return msl::hash_md5(o_key_pad+hash);
}

std::string msl::hmac_sha160(std::string key,const std::string& plain)
{
	if(key.size()>SHA160_BLOCK_SIZE)
		key=msl::hash_sha160(key);

	std::string o_key_pad(SHA160_BLOCK_SIZE,0x5c);
	std::string i_key_pad(SHA160_BLOCK_SIZE,0x36);

	for(size_t ii=0;ii<key.size();++ii)
	{
		o_key_pad[ii]^=key[ii];
		i_key_pad[ii]^=key[ii];
	}

	std::string hash=msl::hash_sha160(i_key_pad+plain);
	return msl::hash_sha160(o_key_pad+hash);
}

std::string msl::hmac_sha256(std::string key,const std::string& plain)
{
	if(key.size()>SHA256_BLOCK_SIZE)
		key=msl::hash_sha256(key);

	std::string o_key_pad(SHA256_BLOCK_SIZE,0x5c);
	std::string i_key_pad(SHA256_BLOCK_SIZE,0x36);

	for(size_t ii=0;ii<key.size();++ii)
	{
		o_key_pad[ii]^=key[ii];
		i_key_pad[ii]^=key[ii];
	}

	std::string hash=msl::hash_sha256(i_key_pad+plain);
	return msl::hash_sha256(o_key_pad+hash);
}

std::string msl::hmac_sha512(std::string key,const std::string& plain)
{
	if(key.size()>SHA512_BLOCK_SIZE)
		key=msl::hash_sha512(key);

	std::string o_key_pad(SHA512_BLOCK_SIZE,0x5c);
	std::string i_key_pad(SHA512_BLOCK_SIZE,0x36);

	for(size_t ii=0;ii<key.size();++ii)
	{
		o_key_pad[ii]^=key[ii];
		i_key_pad[ii]^=key[ii];
	}

	std::string hash=msl::hash_sha512(i_key_pad+plain);
	return msl::hash_sha512(o_key_pad+hash);
}

std::string msl::pbkdf2(const std::string& password,const std::string& salt,const size_t key_byte_size,
	const size_t iterations)
{
	std::string key;
	key.resize(key_byte_size);

	if(PKCS5_PBKDF2_HMAC_SHA1(password.data(),password.size(),
		(uint8_t*)salt.data(),salt.size(),iterations,key_byte_size,(uint8_t*)key.data())==0)
		throw msl::hash_error("msl::pbkdf2() - PKCS5_PBKDF2_HMAC_SHA1 failed.");

	return key;
}