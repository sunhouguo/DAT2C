#pragma once
#include "privatekey.h"

namespace DigitalSignature {

class CSm2PrivateKey :
	public CPrivateKey
{
public:
	CSm2PrivateKey(std::string keyPath, std::string passwd);
	CSm2PrivateKey(std::string keyPath);
	virtual ~CSm2PrivateKey(void);

	virtual bool ValidKey();

private:
	virtual int getKeyLength();
	virtual int Sign(const unsigned char *dgst, int dgstlen,unsigned char *sig, unsigned int *siglen);

	int LoadKey(std::string strBigNum, const char * passwd);
	int FreeKey();
};

};//namespace DigitalSignature

