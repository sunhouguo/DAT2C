#pragma once
#include <openssl/rsa.h>
#include "PrivateKey.h"

namespace DigitalSignature {

class CRsaPrivateKey
	:public CPrivateKey
{
public:
	CRsaPrivateKey(std::string keypath, std::string passwd);
	CRsaPrivateKey(std::string keypath);
	virtual ~CRsaPrivateKey(void);

	virtual bool ValidKey();
	
private:
	virtual int getKeyLength();
	virtual int Sign(const unsigned char *dgst, int dgstlen,unsigned char *sig, unsigned int *siglen);

	int LoadKey(std::string keypath, const char * passwd);
	int FreeKey();

private:
	RSA * key_;
};

};//namespace DigitalSignature 
