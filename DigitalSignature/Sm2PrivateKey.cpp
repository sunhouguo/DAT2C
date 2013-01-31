#include <sstream>
#include <iomanip>
#include <iostream>

#include "../PublicSupport/Dat2cPublicAPI.h"
#include "Sm2PrivateKey.h"

namespace DigitalSignature {

const unsigned char bn_line_index = 1;
const size_t DI_SIGN_LENGTH = 96;

using namespace std;

CSm2PrivateKey::CSm2PrivateKey(std::string keypath, std::string passwd)
{
	if (passwd.empty())
	{
		LoadKey(getFileStr(keypath,bn_line_index),NULL);
	}
	else
	{
		LoadKey(getFileStr(keypath,bn_line_index),passwd.c_str());
	}
}

CSm2PrivateKey::CSm2PrivateKey(std::string keypath)
{
	LoadKey(getFileStr(keypath,bn_line_index),NULL);
}

CSm2PrivateKey::~CSm2PrivateKey(void)
{
	FreeKey();
}

int CSm2PrivateKey::getKeyLength()
{
	return DI_SIGN_LENGTH;
}

bool CSm2PrivateKey::ValidKey()
{
	return true;
}

 //装载私钥
//************************************
// Method:    LoadKey
// FullName:  CECC::LoadKey
// Access:    private 
// Returns:   int
// Qualifier:
// Parameter: std::string keypath 私钥的大数输入
// Parameter: std::string passwd 私钥文件的保护口令，没有则写NULL
// Return:    0：成功 其它：失败
//************************************
int CSm2PrivateKey::LoadKey( std::string strBigNum, const char * passwd )
{
	return 0;
}

//************************************
// Method:    FreeKey
// FullName:  CECC::FreeKey
// Access:    private 
// Returns:   int
// Qualifier:
// Return:    0：成功 其它：失败
//************************************
int CSm2PrivateKey::FreeKey()
{
	return 0;
}

//************************************
// Method:    Sign
// FullName:  CECC::Sign
// Access:    private 
// Returns:   int
// Qualifier:
// Parameter: const unsigned char * dgst 签名输入数据
// Parameter: int dgstlen 签名输入数据长度
// Parameter: unsigned char * sig 签名输出缓冲区
// Parameter: unsigned int * siglen 签名输出数据长度
// Return:    0：成功 其它：失败
//************************************
int CSm2PrivateKey::Sign(const unsigned char *dgst, int dgstlen,unsigned char *sig, unsigned int *siglen)
{
	return 0;
}

};//namespace DigitalSignature
