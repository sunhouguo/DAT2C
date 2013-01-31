#include <boost/algorithm/string/predicate.hpp>
#include "KeyFactory.h"
#include "EccPrivateKey.h"
#include "EccPublicKey.h"
#include "RsaPrivateKey.h"
#include "RsaPublicKey.h"
#include "SpeEccPrivateKey.h"
#include "SpeEccPublicKey.h"
#include "Sm2PublicKey.h"
#include "Sm2PrivateKey.h"
#include "EvpKey.h"

namespace DigitalSignature {

#define strEccDigitalSignature "Ecc"  //使用ECC方式进行加密
#define strRsaDigitalSignature "Rsa"  //使用RSA方式进行加密
#define strSpeEccDigitalSignature "SpeEcc"//特殊ECC加密方式
#define strSm2DigitalSignature "Sm2"  //使用sm2加密方式

const unsigned char NoEncryption = 0;                      //不签名
const unsigned char EccDigitalSignature = 1;               //ECC数字签名加密
const unsigned char RsaDigitalSignature = 2;               //RSA数字签名加密
const unsigned char SpeEccDigitalSignature = 3;            //特殊ECC数字签名加密，长沙使用
const unsigned char Sm2DigitalSignature = 4;               //使用sm2加密方式

CKeyFactory::CKeyFactory(void)
{
}

CKeyFactory::~CKeyFactory(void)
{
}

std::string CKeyFactory::TransSecretKeyTypeToString(unsigned char val)
{
	std::string strRet = "";

	switch(val)
	{
	case EccDigitalSignature:
		strRet = strEccDigitalSignature;
		break;

	case RsaDigitalSignature:
		strRet = strRsaDigitalSignature;
		break;

	case SpeEccDigitalSignature:
		strRet = strSpeEccDigitalSignature;
		break;

	case Sm2DigitalSignature:
		strRet = strSm2DigitalSignature;
		break;

	default:
		break;
	}

	return strRet;
}

unsigned char CKeyFactory::TransSecretKeyTypeFromString(std::string val)
{
	unsigned char ret = 0;

	if (boost::iequals(val,strEccDigitalSignature))
	{
		ret = EccDigitalSignature;	
	}
	else if (boost::iequals(val,strRsaDigitalSignature))
	{
		ret = RsaDigitalSignature;	
	}
	else if (boost::iequals(val,strSpeEccDigitalSignature))
	{
		ret = SpeEccDigitalSignature;	
	}
	else if (boost::iequals(val,strSm2DigitalSignature))
	{
		ret = Sm2DigitalSignature;
	}
	else
	{
		ret = NoEncryption;
	}

	return ret;
}

std::string CKeyFactory::TransEvpKeyTypeToString(unsigned char val)
{
	std::string strRet = "";

	return strRet;
}

unsigned char CKeyFactory::TransEvpKeyTypeFromString(std::string val)
{
	unsigned char ret = NoEncryption;

	return ret;
}

CPrivateKey * CKeyFactory::CreatePrivateKey(std::string keytype,std::string keypath)
{
	unsigned char ret = TransSecretKeyTypeFromString(keytype);

	CPrivateKey * key = NULL;

	switch(ret)
	{
	case EccDigitalSignature:
		key = new CEccPrivateKey(keypath);
		break;

	case RsaDigitalSignature:
		key = new CRsaPrivateKey(keypath);
		break;

	case SpeEccDigitalSignature:
		key = new CSpeEccPrivateKey(keypath);
		break;

	case Sm2DigitalSignature:
		key = new CSm2PrivateKey(keypath);
		break;

	default:
		break;
	}

	return key;
}

CPublicKey * CKeyFactory::CreatePublicKey(std::string keytype,std::string keypath,Protocol::CProtocol & pl)
{
	unsigned char ret = TransSecretKeyTypeFromString(keytype);

	CPublicKey * key = NULL;

	switch(ret)
	{
	case EccDigitalSignature:
		key = new CEccPublicKey(keypath);
		break;

	case RsaDigitalSignature:
		key = new CRsaPublicKey(keypath);
		break;

	case SpeEccDigitalSignature:
		key = new CSpeEccPublicKey(keypath);
		break;

	case Sm2DigitalSignature:
		key = new CSm2PublicKey(keypath,pl);
		break;

	default:
		break;
	}

	return key;
}

CEvpKey * CKeyFactory::CreateEvpKey(std::string keytype,std::string keypath)
{
	unsigned char ret = TransEvpKeyTypeFromString(keytype);

	CEvpKey * key = NULL;

	switch(ret)
	{
	case EccDigitalSignature:
		break;

	default:
		key = new CEvpKey();
		break;
	}

	return key;
}

}; //namespace DigitalSignature
