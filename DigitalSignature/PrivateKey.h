#pragma once

namespace DigitalSignature {

class CPrivateKey
{
public:
	CPrivateKey(void);
	virtual ~CPrivateKey(void);

	virtual int AssembleSignature(const unsigned char * inBuf, int inL, unsigned char * outBuf, int & outL);	// ×é×°Ç©Ãû
	virtual bool ValidKey() = 0;

private:
	virtual int getKeyLength() = 0;
	virtual int Sign(const unsigned char *dgst, int dgstlen,unsigned char *sig, unsigned int *siglen) = 0;
};

}; //namespace DigitalSignature

