#pragma once
#include <openssl/ossl_typ.h>
#include "PublicKey.h"

namespace DigitalSignature {

class CSpeEccPublicKey
	:public CPublicKey
{
public:
	CSpeEccPublicKey(std::string keypath);
	virtual ~CSpeEccPublicKey(void);

	virtual bool ValidKey();
	virtual int getKeyLength();

	virtual int ParseSignature(unsigned char * inBuf, int inL, int dsIndex,unsigned char * outBuf, int & outL); //½âÎöÇ©Ãû

private:
	virtual int Authentic(unsigned char *dgst, int dgstlen,unsigned char *sig, int siglen);

	int LoadKey(std::string keypath);
	int FreeKey();

private:
	EC_KEY * key_;
};

};//namespace DigitalSignature 


