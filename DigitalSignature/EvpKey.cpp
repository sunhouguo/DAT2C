#include "EvpKey.h"
#include <cstring>

namespace DigitalSignature {

CEvpKey::CEvpKey(void)
{
}

CEvpKey::~CEvpKey(void)
{
}

int CEvpKey::encrypt(const unsigned char * src,int srcLength,unsigned char * dst)
{
	memcpy(dst,src,srcLength);

	return srcLength;
}

int CEvpKey::decrypt(const unsigned char * src,int srcLength,unsigned char * dst)
{
	memcpy(dst,src,srcLength);

	return srcLength;
}

};//namespace DigitalSignature
