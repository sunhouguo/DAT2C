#pragma once
#include <openssl/ossl_typ.h>
#include "PublicKey.h"

namespace DigitalSignature {

class CEccPublicKey
	:public CPublicKey
{
public:
	CEccPublicKey(std::string keypath);
	virtual ~CEccPublicKey(void);

	virtual bool ValidKey();
	virtual int getKeyLength();

	virtual int ParseSignature(unsigned char * inBuf, int inL, int dsIndex,unsigned char * outBuf, int & outL); //½âÎöÇ©Ãû
	virtual int CalcSecretDataLength(unsigned char * buf,bool bCalcByFrame);
	
private:
	virtual int Authentic(unsigned char *dgst, int dgstlen,unsigned char *sig, int siglen);

	int LoadKey(std::string keypath);
	int FreeKey();

private:
	EVP_PKEY * key_;
};

};//namespace DigitalSignature 

