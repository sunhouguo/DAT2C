#pragma once

namespace DigitalSignature {

class CEvpKey
{
public:
	CEvpKey(void);
	virtual ~CEvpKey(void);

	virtual int encrypt(const unsigned char * src,int srcLength,unsigned char * dst);
	virtual int decrypt(const unsigned char * src,int srcLength,unsigned char * dst);
};

};//namespace DigitalSignature

