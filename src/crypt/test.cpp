/*
 * test.cpp
 *
 *  Created on: Sep 22, 2017
 *      Author: hongxu
 */
#include "Logger.h"
#include "CCrypt.h"
#include <iostream>
#include <string.h>
#include <openssl/rand.h>
using namespace std;


string g_private_key = "-----BEGIN RSA PRIVATE KEY-----\n\
MIICXgIBAAKBgQDBB73Ug61gDtXTGJN0VE+JZckCq4d0yCzNw6X8Oy92iY38qgMs\n\
9/nAQvUhEP3wSFwy2Z5hhMD8+0er3wdaVEPZj6VERSiJs1dQEs7OoyHdMrff4ZXl\n\
oufW93SQ5CYC1/PPah0ShIvjKMmQZkFXPS9G0mJhtRGT/n8X97VyMMCl3QIDAQAB\n\
AoGAQJeAlB5D48LTFkJBBUo5GzbuHnCbcR6Sr3/qiQ6dAUNiOcwCKAgKkKVXNWtk\n\
LgVopLNhZixwD7dd8ks9QKK12P9c0R+W64bNUD6+1jFPIFNCN+cyv8/MN4+bfpuC\n\
kbQ2TB2Mi9SGwbVZMuHO0GaTtgEOR2joFyfr1SBakFt1PlkCQQD8NBiAXXGXtysW\n\
RGt5U9PRB+P2dcay7ktnDY6oChf4E/90MIDYs899bvoy/XvqfYeWndQk3MgNwRlS\n\
7U490d2PAkEAw++c216QcRDaBL8AJZw94nCbULlb3eFpbsswMX7rAWaLTIKsmegd\n\
DbpClTWW2nS2DJSF0lSwHFHK25qs7PXn0wJBAN9Cvu3jyssju2Da8MlXDAvfkE59\n\
tIhxqw5vUfov9UgHa/zPc0Bi7St9MsAw5aGPvpf23/ZB/FcwudpPeRQA1MECQQCj\n\
+qUL8thvx+qujitRTrF8EUHrDTYVO9FBKEvtIIsNYQSzw6kwJVIravbOaUsXCRZS\n\
8PoOTVIWYV2k7I5MwoCBAkEAzxGfjiGb5gGJVVIbzwiEFKYBZQ8rUlkA5v2/MH2C\n\
JJuZNrMAIxAKvLDU7+ky0JVkmGxcV5sVZdgu0rliunkxuw==\n\
-----END RSA PRIVATE KEY-----";

string g_public_key = "-----BEGIN PUBLIC KEY-----\n\
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDBB73Ug61gDtXTGJN0VE+JZckC\n\
q4d0yCzNw6X8Oy92iY38qgMs9/nAQvUhEP3wSFwy2Z5hhMD8+0er3wdaVEPZj6VE\n\
RSiJs1dQEs7OoyHdMrff4ZXloufW93SQ5CYC1/PPah0ShIvjKMmQZkFXPS9G0mJh\n\
tRGT/n8X97VyMMCl3QIDAQAB\n\
-----END PUBLIC KEY-----";


void testRSA()
{
	CCrypt c;

	c.setRSAPrivateKey(g_private_key);

	c.setRSAPublicKey(g_public_key);


	const char text[18] = "everything is ok.";

	string tmp(text, sizeof(text));
	string en = c.rsaPrivateEncrypt(tmp);

	cout << "Encrypt:" << endl;
	cout << en << endl;
	cout << "---------------" << endl;

	string de = c.rsaPublicDecrypt(en);

	cout << "Decrypt:" << endl;
 	cout << de << endl;
 	cout << "---------------" << endl;
}


void testDes()
{
	CCrypt c;
	c.gen3DesKey();

	const char *src = "123";
	cout << strlen(src) << endl;

	string t = src;
	string en = c.desEncrypt(t);
	cout << en << endl;

	string de = c.desDecrypt(en);
	cout << de << endl;
	cout << de.size() << endl;
}


void testRand()
{
	unsigned char buf[8];
	RAND_bytes(buf, sizeof(buf));

	for(int i = 0; i < sizeof(buf)/sizeof(buf[0]); i++)
	{
		cout << (int)(buf[i]) << " ";
	}
	cout << endl;
}

void testDigest()
{
	CCrypt c;
	const char *p = "1234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353123465623454352435243531234656234543524352435312346562345435243524353";

	string t = p;
	string d = c.digest(t);

    printf("Digest(%lu) is: ", d.size());
    for (uint32_t i = 0; i < d.size(); i++)
        printf("%02x", (uint8_t)d[i]);
    printf("\n");
}


int main()
{
	initLogger("testlog.cfg");

	testDigest();

	return 0;
}



