#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstring>

#if defined(__linux__)
#include <netinet/in.h>
#endif

#if defined(_WIN32)
#include <winsock2.h>
#endif

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/md5.h>

#include "../PublicSupport/Dat2cPublicAPI.h"
#include "EccPrivateKey.h"

namespace DigitalSignature {

const char rnd_seed[] = "string to make the random number generator think it has entropy";
const size_t DI_SIGN_LENGTH = 96;
const unsigned char DI_SIGN_START_TAG = 0x16;

using namespace std;

CEccPrivateKey::CEccPrivateKey(std::string keypath, std::string passwd)
{
	key_ = NULL;

	if (passwd.empty())
	{
		LoadKey(keypath,NULL);
	}
	else
	{
		LoadKey(keypath,passwd.c_str());
	}
}

CEccPrivateKey::CEccPrivateKey(std::string keypath)
{
	key_ = NULL;

	LoadKey(keypath,NULL);
}

CEccPrivateKey::~CEccPrivateKey(void)
{
	FreeKey();
}

int CEccPrivateKey::getKeyLength()
{
	return DI_SIGN_LENGTH;
}

bool CEccPrivateKey::ValidKey()
{
	if (key_)
	{
		return EC_KEY_check_key(key_) == 1;
	}

	return false;
}

 //装载私钥
//************************************
// Method:    LoadKey
// FullName:  CECC::LoadKey
// Access:    private 
// Returns:   int
// Qualifier:
// Parameter: std::string keypath 私钥文件路径
// Parameter: std::string passwd 私钥文件的保护口令，没有则写NULL
// Return:    0：成功 其它：失败
//************************************
int CEccPrivateKey::LoadKey( std::string keypath, const char * passwd )
{
	RAND_seed(rnd_seed, sizeof(rnd_seed)); /* or OAEP may fail */
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
	ERR_load_crypto_strings();

	BIO * bio = BIO_new_file(keypath.c_str(),"rb");
	if (!bio) 
	{
		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	key_ = PEM_read_bio_ECPrivateKey(bio,NULL,NULL,(void *)passwd);
	if (!key_) 
	{
		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	if (!ValidKey()) 
	{
		FreeKey();

		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	BIO_free(bio);
	
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
int CEccPrivateKey::FreeKey()
{
	if(key_)
	{
		EC_KEY_free(key_);
	}

	key_ = NULL;

	ERR_free_strings();

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
int CEccPrivateKey::Sign(const unsigned char *dgst, int dgstlen,unsigned char *sig, unsigned int *siglen)
{
	if (!key_)
	{
		ostringstream ostr;
		ostr<<"密钥未能装载"<<endl;
		cerr<<ostr.str();

		return -1;
	}

	int ret = ECDSA_sign(0,dgst,dgstlen,sig,siglen,key_);
	if (ret != 1) 
	{
		ostringstream ostr;
		ostr<<"ECDSA_sign: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();

		return -1;
	}

	return 0;
}

//************************************
// Method:    AssembleSignature
// FullName:  CECC::AssembleSignature
// Access:    public 
// Returns:   int
// Qualifier: 组装签名 out = 原始报文|16H|L2|C|S|Timestamp(4bytes)|Label|签名数据 L2: 之后的报文长度(包括控制标志C，保留字节S，安全标签Label和签名数据的长度)
// Parameter: unsigned char * inBuf 原始报文
// Parameter: int inL 原始报文总长度
// Parameter: unsigned char * outBuf 返回签名后的报文
// Parameter: int & outL 输入为out缓冲区的大小，输出签名后的整个报文的大小
// Return:    0：成功 其它：失败
//************************************
int CEccPrivateKey::AssembleSignature(const unsigned char * inBuf, int inL, unsigned char * outBuf, int & outL)
{
	using namespace std;

	// 参数检查
	if (!ValidKey())
	{
		ostringstream ostr;
		ostr<<"私有密钥未能装载"<<endl;
		cerr<<ostr.str();

		return -1;
	}

	if (!inBuf || !outBuf)
	{
		ostringstream ostr;
		ostr<<"输入或者输出缓冲区指针非法"<<endl;
		cerr<<ostr.str();

		return -1;
	}

	if (outL - inL < getKeyLength())
	{
		ostringstream ostr;
		ostr<<"输出缓冲区过小 outL = "<<outL<<" inL = "<<inL<<",Need (outL - inL) >= "<<getKeyLength()<<endl;
		cerr<<ostr.str();

		return -1;
	}

	// 开始
	if (outBuf != inBuf)
	{
		memcpy(outBuf,inBuf,inL);
	}

	int count = inL;
	outBuf[count++] = DI_SIGN_START_TAG; // 附加内容开始标志
	outBuf[count++] = 0;    // 长度，先置0
	outBuf[count++] = 0;    // C标志，暂时置0
	outBuf[count++] = 0;    // S标志，暂时置0
	count += ValToBuf(&outBuf[count],htonl((u_long)time(NULL)),4); // 时间戳
	memset(&outBuf[count],0,16);
	count += 16; // 安全标签，先跳过

	MD5_CTX md5;
	unsigned char hashed[16];
	MD5_Init(&md5);
	MD5_Update(&md5,outBuf,count);
	MD5_Final(hashed,&md5);

	cout<<"Prv hash buf:";
	for (int i=0;i<count;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)outBuf[i];
	}
	cout<<endl;

	cout<<"Prv hash val:";
	for (int i=0;i<16;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)hashed[i];
	}
	cout<<endl;

	unsigned int sig_len;
	int ret = Sign(hashed,sizeof(hashed),&outBuf[count],&sig_len);
	if (ret) 
	{
		ostringstream ostr;
		ostr<<"签名失败"<<endl;
		cerr<<ostr.str();

		return -1;
	}

	cout<<"Prv sign val:";
	for (int i=0;i<(int)sig_len;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)outBuf[count + i];
	}
	cout<<endl;

	int AddFrameLength = count - inL + sig_len;
	outBuf[inL + 1] = AddFrameLength - 2;
	outL = inL + AddFrameLength;

	return 0;
}

};//namespace DigitalSignature

