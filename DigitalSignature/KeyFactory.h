#pragma once
#include <iostream>

namespace Protocol
{
	class CProtocol;
}

namespace DigitalSignature {

class CPrivateKey;
class CPublicKey;
class CEvpKey;

class CKeyFactory
{
public:
	virtual ~CKeyFactory(void);
	static CPrivateKey * CreatePrivateKey(std::string keytype,std::string keypath);
	static CPublicKey * CreatePublicKey(std::string keytype,std::string keypath,Protocol::CProtocol & pl);
	static CEvpKey * CreateEvpKey(std::string keytype,std::string keypath);

private:
	CKeyFactory(void);

	static std::string TransSecretKeyTypeToString(unsigned char val);
	static unsigned char TransSecretKeyTypeFromString(std::string val);

	static std::string TransEvpKeyTypeToString(unsigned char val);
	static unsigned char TransEvpKeyTypeFromString(std::string val);
};

}; //namespace DigitalSignature
