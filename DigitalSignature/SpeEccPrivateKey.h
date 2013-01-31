#pragma once
#include <openssl/ec.h>
#include "PrivateKey.h"

namespace DigitalSignature {

class CSpeEccPrivateKey
	:public CPrivateKey
{
public:
	CSpeEccPrivateKey(std::string keypath, std::string passwd);
	CSpeEccPrivateKey(std::string keypath);
	virtual ~CSpeEccPrivateKey(void);

	virtual bool ValidKey();
	virtual int AssembleSignature(const unsigned char * inBuf, int inL, unsigned char * outBuf, int & outL);	// ×é×°Ç©Ãû
	
private:
	virtual int getKeyLength();
	virtual int Sign(const unsigned char *dgst, int dgstlen,unsigned char *sig, unsigned int *siglen);

	int LoadKey(std::string keypath, const char * passwd);
	int FreeKey();

private:
	EC_KEY * key_;
};

};//namespace DigitalSignature 

