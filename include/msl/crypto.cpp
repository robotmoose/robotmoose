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
#define SHA256_BLOCK_SIZE				64
#define SHA512_BLOCK_SIZE				128

bool msl::encrypt_rsa(const void* plain,const size_t size,const std::string& key,std::string& cipher)
{
	bool success=true;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	BIO* keybio=BIO_new_mem_buf((uint8_t*)key.c_str(),-1);

	if(keybio==nullptr)
		success=false;

	RSA* rsa=nullptr;

	if(success)
		rsa=PEM_read_bio_RSA_PUBKEY(keybio,&rsa,nullptr,nullptr);

	if(rsa==nullptr)
		success=false;

	if(size>(size_t)RSA_size(rsa)-RSA_PKCS1_OAEP_PADDING_SIZE)
		success=false;

	size_t temp_size=(size_t)-1;
	uint8_t* temp_data=new uint8_t[RSA_size(rsa)];

	if(success)
		temp_size=RSA_public_encrypt(size,(uint8_t*)plain,temp_data,rsa,RSA_PKCS1_OAEP_PADDING);

	if(success&&temp_size==(size_t)~0)
		success=false;

	if(success)
		cipher=std::string((char*)temp_data,temp_size);

	delete[] temp_data;
	BIO_free(keybio);
	RSA_free(rsa);
	ERR_free_strings();
	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return success;
}

bool msl::encrypt_rsa(const std::string& plain,const std::string& key,std::string& cipher)
{
	return msl::encrypt_rsa(plain.c_str(),plain.size(),key,cipher);
}

bool msl::encrypt_aes256(const void* plain,const size_t size,const std::string& key,const std::string& iv,std::string& cipher)
{
	bool success=false;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	EVP_CIPHER_CTX* ctx=EVP_CIPHER_CTX_new();
	uint8_t* temp_data=new uint8_t[(size/16+1)*16];

	int temp_length;
	int temp_unaligned_length;

	if(ctx!=nullptr&&EVP_CIPHER_CTX_set_padding(ctx,1)!=0&&
		EVP_EncryptInit_ex(ctx,EVP_aes_256_cbc(),nullptr,(uint8_t*)key.c_str(),(uint8_t*)iv.c_str())!=0&&
		EVP_EncryptUpdate(ctx,temp_data,&temp_length,(uint8_t*)plain,size)!=0&&
		EVP_EncryptFinal_ex(ctx,temp_data+temp_length,&temp_unaligned_length)!=0)
	{
		cipher=std::string((char*)temp_data,temp_length+temp_unaligned_length);
		success=true;
	}

	delete[] temp_data;
	EVP_CIPHER_CTX_free(ctx);
	ERR_free_strings();
	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return success;
}

bool msl::encrypt_aes256(const std::string& plain,const std::string& key,const std::string& iv,std::string& cipher)
{
	return encrypt_aes256(plain.c_str(),plain.size(),key,iv,cipher);
}

bool msl::decrypt_rsa(const void* cipher,const size_t size,const std::string& key,std::string& plain)
{
	bool success=true;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	BIO* keybio=BIO_new_mem_buf((uint8_t*)key.c_str(),-1);

	if(keybio==nullptr)
		success=false;

	RSA* rsa=nullptr;

	if(success)
		rsa=PEM_read_bio_RSAPrivateKey(keybio,&rsa,nullptr,nullptr);

	if(rsa==nullptr)
		success=false;

	if(size>(size_t)RSA_size(rsa))
		success=false;

	size_t temp_size=(size_t)-1;
	uint8_t* temp_data=new uint8_t[RSA_size(rsa)];

	if(success)
		temp_size=RSA_private_decrypt(size,(uint8_t*)cipher,temp_data,rsa,RSA_PKCS1_OAEP_PADDING);

	if(temp_size==(size_t)~0)
		success=false;

	if(success)
		plain=std::string((char*)temp_data,temp_size);

	delete[] temp_data;
	BIO_free(keybio);
	RSA_free(rsa);
	ERR_free_strings();
	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return success;
}

bool msl::decrypt_rsa(const std::string& cipher,const std::string& key,std::string& plain)
{
	return msl::decrypt_rsa(cipher.c_str(),cipher.size(),key,plain);
}

bool msl::decrypt_aes256(const void* cipher,const size_t size,const std::string& key,const std::string& iv,std::string& plain)
{
	bool success=false;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	EVP_CIPHER_CTX* ctx=EVP_CIPHER_CTX_new();
	uint8_t* temp_data=new uint8_t[(size/16+1)*16];

	int temp_length;
	int temp_unaligned_length;

	if(ctx!=nullptr&&
		EVP_DecryptInit_ex(ctx,EVP_aes_256_cbc(),nullptr,(uint8_t*)key.c_str(),(uint8_t*)iv.c_str())!=0&&
		EVP_DecryptUpdate(ctx,temp_data,&temp_length,(uint8_t*)cipher,size)!=0&&
		EVP_DecryptFinal_ex(ctx,temp_data+temp_length,&temp_unaligned_length)!=0)
	{
		plain=std::string((char*)temp_data,temp_length+temp_unaligned_length);
		success=true;
	}

	delete[] temp_data;
	EVP_CIPHER_CTX_free(ctx);
	ERR_free_strings();
	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return success;
}

bool msl::decrypt_aes256(const std::string& cipher,const std::string& key,const std::string& iv,std::string& plain)
{
	return msl::decrypt_aes256(cipher.c_str(),cipher.size(),key,iv,plain);
}

bool msl::hash_md5(const std::string& plain,std::string& hash)
{
	std::string temp_hash;
	temp_hash.resize(MD5_DIGEST_LENGTH);

	MD5_CTX ctx;

	if(MD5_Init(&ctx)==1&&MD5_Update(&ctx,(unsigned char*)plain.c_str(),plain.size())==1&&
		MD5_Final((unsigned char*)temp_hash.data(),&ctx)==1)
	{
		hash=temp_hash;
		return true;
	}

	return false;
}

bool msl::hash_sha256(const std::string& plain,std::string& hash)
{
	std::string temp_hash;
	temp_hash.resize(SHA256_DIGEST_LENGTH);

	SHA256_CTX ctx;

	if(SHA256_Init(&ctx)==1&&SHA256_Update(&ctx,(unsigned char*)plain.c_str(),plain.size())==1&&
		SHA256_Final((unsigned char*)temp_hash.data(),&ctx)==1)
	{
		hash=temp_hash;
		return true;
	}

	return false;
}

bool msl::hash_sha512(const std::string& plain,std::string& hash)
{
	std::string temp_hash;
	temp_hash.resize(SHA512_DIGEST_LENGTH);

	SHA512_CTX ctx;

	if(SHA512_Init(&ctx)==1&&SHA512_Update(&ctx,(unsigned char*)plain.c_str(),plain.size())==1&&
		SHA512_Final((unsigned char*)temp_hash.data(),&ctx)==1)
	{
		hash=temp_hash;
		return true;
	}

	return false;
}

bool msl::hmac_md5(std::string key,const std::string& plain,std::string& hash)
{
	if(key.size()>MD5_BLOCK_SIZE)
	{
		if(!msl::hash_md5(key,key))
			return false;
	}

	std::string o_key_pad(MD5_BLOCK_SIZE,0x5c);
	std::string i_key_pad(MD5_BLOCK_SIZE,0x36);

	for(size_t ii=0;ii<key.size();++ii)
	{
		o_key_pad[ii]^=key[ii];
		i_key_pad[ii]^=key[ii];
	}

	std::string hash_temp="";

	if(!msl::hash_md5(i_key_pad+plain,hash_temp)||!msl::hash_md5(o_key_pad+hash_temp,hash_temp))
		return false;

	hash=hash_temp;
	return true;
}

bool msl::hmac_sha256(std::string key,const std::string& plain,std::string& hash)
{
	if(key.size()>SHA256_BLOCK_SIZE)
	{
		if(!msl::hash_sha256(key,key))
			return false;
	}

	std::string o_key_pad(SHA256_BLOCK_SIZE,0x5c);
	std::string i_key_pad(SHA256_BLOCK_SIZE,0x36);

	for(size_t ii=0;ii<key.size();++ii)
	{
		o_key_pad[ii]^=key[ii];
		i_key_pad[ii]^=key[ii];
	}

	std::string hash_temp="";

	if(!msl::hash_sha256(i_key_pad+plain,hash_temp)||!msl::hash_sha256(o_key_pad+hash_temp,hash_temp))
		return false;

	hash=hash_temp;
	return true;
}

bool msl::hmac_sha512(std::string key,const std::string& plain,std::string& hash)
{
	if(key.size()>SHA512_BLOCK_SIZE)
	{
		if(!msl::hash_sha512(key,key))
			return false;
	}

	std::string o_key_pad(SHA512_BLOCK_SIZE,0x5c);
	std::string i_key_pad(SHA512_BLOCK_SIZE,0x36);

	for(size_t ii=0;ii<key.size();++ii)
	{
		o_key_pad[ii]^=key[ii];
		i_key_pad[ii]^=key[ii];
	}

	std::string hash_temp="";

	if(!msl::hash_sha512(i_key_pad+plain,hash_temp)||!msl::hash_sha512(o_key_pad+hash_temp,hash_temp))
		return false;

	hash=hash_temp;
	return true;
}