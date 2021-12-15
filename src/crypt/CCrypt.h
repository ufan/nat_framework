/*
 * CCrypt.h
 *
 *  Created on: Sep 22, 2017
 *      Author: hongxu
 */

#ifndef CRYPT_CCRYPT_H_
#define CRYPT_CCRYPT_H_

#include <openssl/des.h>
#include <stdint.h>

#include <string>
using namespace std;

const uint8_t g_xorkey[] = {0x20, 0x17, 0x10, 0x09, 0xff, 0xff,
                            0xff, 0xff, 0x13, 0x48, 0x88, 0x88,
                            0x88, 0x88, 0x88, 0x88, 0x88, 0x88};

class CCrypt {
 public:
  CCrypt();
  virtual ~CCrypt();

  void setRSAPublicKey(const string &k) { rsa_public_key_ = k; }
  void setRSAPrivateKey(const string &k) { rsa_private_key_ = k; }

  void xorDecryptKey(string &key);

  /*
     source length cannot exceeds RSA_size(rsa) - 11, see the openssl document
     for detail. for 1024 bit key, maximum length is 128 - 11 = 117
  */
  string rsaPublicEncrypt(string &source);     // use puk to encrypt
  string rsaPublicDecrypt(string &encrypted);  // use pub to decrypt

  string rsaPrivateEncrypt(string &source);     // use prk to encrypt
  string rsaPrivateDecrypt(string &encrypted);  // use prk to decrypt

  void gen3DesKey();  // generate DES keys internally
  const char *get3DesKeyBlock() { return (const char *)deskey_; }
  void set3DesKey(const char *p);  // set DES keys from external source

  // 3DES encryption and decryption
  string desEncrypt(string &source);
  string desDecrypt(string &source);

  string digestFile(int fd);
  string digest(string &source);

  static void init();

 private:
  string rsa_public_key_;
  string rsa_private_key_;

  DES_cblock deskey_[3];
  DES_key_schedule ks_[3];

  static string s_xor_key_;
};

#define KEY_SIZE_OF_3DES (sizeof(DES_cblock) * 3)

#endif /* CRYPT_CCRYPT_H_ */
