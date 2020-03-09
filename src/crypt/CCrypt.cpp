/*
 * CCrypt.cpp
 *
 *  Created on: Sep 22, 2017
 *      Author: hongxu
 */

#include "CCrypt.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "compiler.h"
#include "Logger.h"


string CCrypt::s_xor_key_((char*)g_xorkey, sizeof(g_xorkey));


CCrypt::CCrypt()
{
	init();
}

CCrypt::~CCrypt()
{

}

void CCrypt::xorDecryptKey(string &key)
{
	uint32_t len = key.size();
	uint32_t xorlen = s_xor_key_.size();

	uint8_t *p = (uint8_t*)key.data();
	for(uint32_t i = 0, j = 0; i < len; i++)
	{
		*p++ ^= (uint8_t)s_xor_key_[j++];
		if(j == xorlen) j = 0;
	}
}

string CCrypt::rsaPublicEncrypt(string &source)
{
	string enres;
	xorDecryptKey(rsa_public_key_);

	RSA    *rsa = NULL;
	BIO    *bio = NULL;

	if(NULL == (bio = BIO_new_mem_buf(rsa_public_key_.c_str(), -1)))
	{
		xorDecryptKey(rsa_public_key_);
		LOG_ERR("new bio err");
		return enres;
	}

	rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
	xorDecryptKey(rsa_public_key_);
	if (rsa == NULL)
	{
		LOG_ERR("read rsa key err.");
	}
	else
	{
		int rsa_len = RSA_size(rsa);
		char *p_en = new char[rsa_len];
		memset(p_en, 0, rsa_len);

		int iret = RSA_public_encrypt((int)source.size(), (unsigned char*)source.c_str(),
				(unsigned char*)p_en, rsa, RSA_PKCS1_PADDING);
		if(iret < 0)
		{
			LOG_ERR("encrypt err");
		}
		else
		{
			enres.assign(p_en, iret);
		}

		delete []p_en;
	}

	if(rsa) RSA_free(rsa);
	BIO_free_all(bio);
	return enres;
}

string CCrypt::rsaPrivateDecrypt(string &encrypted)
{
	string deres;
	xorDecryptKey(rsa_private_key_);

	RSA    *rsa = NULL;
	BIO    *bio = NULL;

	if(NULL == (bio = BIO_new_mem_buf(rsa_private_key_.c_str(), -1)))
	{
		xorDecryptKey(rsa_private_key_);
		LOG_ERR("new bio err");
		return deres;
	}

	rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
	xorDecryptKey(rsa_private_key_);
	if (rsa == NULL)
	{
		LOG_ERR("read rsa key err.");
	}
	else
	{
		int rsa_len = RSA_size(rsa);
		char *p_de = new char[rsa_len];
		memset(p_de, 0, rsa_len);

		int iret = RSA_private_decrypt((int)encrypted.size(), (unsigned char*)encrypted.c_str(),
				(unsigned char*)p_de, rsa, RSA_PKCS1_PADDING);
		if(iret < 0)
		{
			LOG_ERR("decrypt err");
		}
		else
		{
			deres.assign(p_de, iret);
		}

		delete []p_de;
	}

	if(rsa) RSA_free(rsa);
	BIO_free_all(bio);
	return deres;
}

string CCrypt::rsaPrivateEncrypt(string &source)
{
	string enres;
	xorDecryptKey(rsa_private_key_);

	RSA    *rsa = NULL;
	BIO    *bio = NULL;

	if(NULL == (bio = BIO_new_mem_buf(rsa_private_key_.c_str(), -1)))
	{
		xorDecryptKey(rsa_private_key_);
		LOG_ERR("new bio err");
		return enres;
	}

	rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
	xorDecryptKey(rsa_private_key_);
	if (rsa == NULL)
	{
		LOG_ERR("read rsa key err.");
	}
	else
	{
		int rsa_len = RSA_size(rsa);
		char *p_en = new char[rsa_len];
		memset(p_en, 0, rsa_len);

		int iret = RSA_private_encrypt((int)source.size(), (unsigned char*)source.c_str(),
				(unsigned char*)p_en, rsa, RSA_PKCS1_PADDING);
		if(iret < 0)
		{
			LOG_ERR("encrypt err");
		}
		else
		{
			enres.assign(p_en, iret);
		}

		delete []p_en;
	}

	if(rsa) RSA_free(rsa);
	BIO_free_all(bio);
	return enres;
}

string CCrypt::rsaPublicDecrypt(string &encrypted)
{
	string deres;
	xorDecryptKey(rsa_public_key_);

	RSA    *rsa = NULL;
	BIO    *bio = NULL;

	if(NULL == (bio = BIO_new_mem_buf(rsa_public_key_.c_str(), -1)))
	{
		xorDecryptKey(rsa_public_key_);
		LOG_ERR("new bio err");
		return deres;
	}

	rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
	xorDecryptKey(rsa_public_key_);
	if (rsa == NULL)
	{
		LOG_ERR("read rsa key err.");
	}
	else
	{
		int rsa_len = RSA_size(rsa);
		char *p_de = new char[rsa_len];
		memset(p_de, 0, rsa_len);

		int iret = RSA_public_decrypt((int)encrypted.size(), (unsigned char*)encrypted.c_str(),
				(unsigned char*)p_de, rsa, RSA_PKCS1_PADDING);
		if(iret < 0)
		{
			LOG_ERR("decrypt err");
		}
		else
		{
			deres.assign(p_de, iret);
		}

		delete []p_de;
	}

	if(rsa) RSA_free(rsa);
	BIO_free_all(bio);
	return deres;
}

void CCrypt::gen3DesKey()
{
	DES_random_key(&(deskey_[0]));
	DES_random_key(&(deskey_[1]));
	DES_random_key(&(deskey_[2]));

	DES_set_key_unchecked(&deskey_[0], &ks_[0]);
	DES_set_key_unchecked(&deskey_[1], &ks_[1]);
	DES_set_key_unchecked(&deskey_[2], &ks_[2]);
}

void CCrypt::set3DesKey(const char *p)
{
	memcpy(deskey_, p, sizeof(deskey_));

	DES_set_key_unchecked(&deskey_[0], &ks_[0]);
	DES_set_key_unchecked(&deskey_[1], &ks_[1]);
	DES_set_key_unchecked(&deskey_[2], &ks_[2]);
}

string CCrypt::desEncrypt(string &source)
{
	string res;
	uint32_t len = source.size() + 1;		// +1 for \0
	uint32_t rest_len = len & 7;
	if(rest_len) len += 8 - rest_len;
	res.resize(len);

	char *to = (char*)res.data();
	const char *from = source.c_str();
	for(uint32_t i = (source.size() + 1) / 8; i; --i)
	{
		DES_ecb3_encrypt((const_DES_cblock*)from, (DES_cblock*)to, &ks_[0], &ks_[1], &ks_[2], DES_ENCRYPT);
		from += 8;
		to += 8;
	}

	if(rest_len)
	{
		uint64_t tmp = 0xffffffffffffffff;
		memcpy(&tmp, from, rest_len);
		DES_ecb3_encrypt((const_DES_cblock*)&tmp, (DES_cblock*)to, &ks_[0], &ks_[1], &ks_[2], DES_ENCRYPT);
	}

	return res;
}

string CCrypt::desDecrypt(string &source)
{
	string res;

	if(source.size() < 8) return res;

	res.resize(source.size());

	char *to = (char*)res.data();
	const char *from = source.data();
	for(uint32_t i = source.size() / 8; i; --i)
	{
		DES_ecb3_encrypt((const_DES_cblock*)from, (DES_cblock*)to, &ks_[0], &ks_[1], &ks_[2], DES_DECRYPT);
		from += 8;
		to += 8;
	}

	int cut = 0;
	const char *p = res.data() + res.size();
	for(; cut < 8 && *--p != '\0'; cut++);
	res.resize(res.size() - cut - 1);

	return res;
}

string CCrypt::digest(string &source)
{
    EVP_MD_CTX *mdctx = NULL;
    const EVP_MD *md = NULL;
    unsigned int md_len = 0;
    string res;

    md = EVP_get_digestbyname("SHA256");
    if(unlikely(!md))
    {
    	LOG_ERR("can't get digest algorithmn");
    	return res;
    }

    res.resize(EVP_MAX_MD_SIZE);

    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, source.data(), source.size());
    EVP_DigestFinal_ex(mdctx, (unsigned char*)res.data(), &md_len);
    EVP_MD_CTX_destroy(mdctx);

    res.resize(md_len);
    return res;
}

string CCrypt::digestFile(int fd)
{
	char buf[256];

    EVP_MD_CTX *mdctx = NULL;
    const EVP_MD *md = NULL;
    unsigned int md_len = 0;
    string res;

    md = EVP_get_digestbyname("SHA256");
    if(unlikely(!md))
    {
    	LOG_ERR("can't get digest algorithmn");
    	return res;
    }

    res.resize(EVP_MAX_MD_SIZE);

    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, NULL);

	off_t ori = lseek(fd, 0, SEEK_CUR);
	lseek(fd, 0, SEEK_SET);

	int ret = 0;
	while(0 < (ret = read(fd, buf, sizeof(buf))))
    {
    	EVP_DigestUpdate(mdctx, buf, ret);
    }

	lseek(fd, ori, SEEK_SET);

    EVP_DigestFinal_ex(mdctx, (unsigned char*)res.data(), &md_len);
    EVP_MD_CTX_destroy(mdctx);

    res.resize(md_len);
    return res;
}

void CCrypt::init()
{
	bool s_isinit = false;
	if(!s_isinit)
	{
		s_isinit = true;
		OpenSSL_add_all_algorithms();
	}
}

