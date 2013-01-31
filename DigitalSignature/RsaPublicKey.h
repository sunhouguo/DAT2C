#pragma once
#include "PublicKey.h"
#include <openssl/ossl_typ.h>

namespace DigitalSignature {

class CRsaPublicKey
	:public CPublicKey
{
public:
	CRsaPublicKey(std::string keypath);
	virtual ~CRsaPublicKey(void);

	virtual bool ValidKey();
	virtual int getKeyLength();
	
private:
	virtual int Authentic(unsigned char *dgst, int dgstlen,unsigned char *sig, int siglen);

	int LoadKey(std::string keypath);
	int FreeKey();
	
private:
	EVP_PKEY * key_;
};

};//namespace DigitalSignature 

