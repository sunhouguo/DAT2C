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
#include "EccPublicKey.h"

namespace DigitalSignature {

const char rnd_seed[] = "string to make the random number generator think it has entropy";
const unsigned char DI_SIGN_START_TAG = 0x16;
const unsigned char HEAD_FRAME_LENGTH = 2;

using namespace std;

CEccPublicKey::CEccPublicKey(std::string keypath)
{
	key_ = NULL;

	LoadKey(keypath);
}

CEccPublicKey::~CEccPublicKey(void)
{
	FreeKey();
}

int CEccPublicKey::getKeyLength()
{
	if (key_)
	{
		return EVP_PKEY_size(key_);
	}

	return -1;
}

//装载公钥
//************************************
// Method:    LoadKey
// FullName:  CEccPublicKey::LoadKey
// Access:    private 
// Returns:   int
// Qualifier:
// Parameter: std::string keypath ECC证书文件路径
// Return:    0：成功 其它：失败
//************************************
int CEccPublicKey::LoadKey(std::string keypath)
{
	RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
	ERR_load_crypto_strings();

	// 读入ecc证书
	BIO * bio = BIO_new_file(keypath.c_str(),"rb");
	if (!bio)
	{
		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	X509 * cert = PEM_read_bio_X509(bio,NULL,NULL,NULL);
	if (!cert) 
	{
		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	key_ = X509_get_pubkey(cert);
	if (!key_)
	{
		ostringstream ostr;
		ostr<<"LoadKey: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();
		return -1;
	}

	//cout<<key_->pkey.ec<<endl;

	X509_free(cert);
	BIO_free(bio);

	return 0;
}

//************************************
// Method:    FreeKey
// FullName:  CEccPublicKey::FreeKey
// Access:    private 
// Returns:   int
// Qualifier: 释放内存
// Return:    0：成功 其它：失败
//************************************
int CEccPublicKey::FreeKey()
{
	if (key_)
	{
		EVP_PKEY_free(key_);
	}

	key_ = NULL;

	ERR_free_strings();

	return 0;
}

bool CEccPublicKey::ValidKey()
{
	if (key_)
	{
		return true;
	}

	return false;
}

//************************************
// Method:    Authentic
// FullName:  Authentic
// Access:    public 
// Returns:   int
// Qualifier: 
// Parameter: const unsigned char * dgst 签名输入数据
// Parameter: int dgstlen 签名输入数据长度
// Parameter: const unsigned char * sig 签名
// Parameter: int siglen 签名长度
// Return:    0：认证正确 其它：失败
//************************************
int CEccPublicKey::Authentic(unsigned char *dgst, int dgstlen,unsigned char *sig, int siglen)
{
	RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */

	if (!key_)
	{
		ostringstream ostr;
		ostr<<"can't load key"<<endl;
		cerr<<ostr.str();

		return -1;
	}

	int ret = ECDSA_verify(0,dgst,dgstlen,sig,siglen,key_->pkey.ec);
	if (ret != 1) 
	{
		ostringstream ostr;
		ostr<<"ECDSA_verify: "<<ERR_error_string(ERR_get_error(),NULL)<<endl;
		cerr<<ostr.str();

		return -1;
	}

	return 0;
}

//************************************
// Method:    ParseSignature
// FullName:  Algorithm::CDigitalSignature::ParseSignature
// Access:    public 
// Returns:   int
// Qualifier: /*解析签名 */
// Parameter: const unsigned char * inBuf 附加签名的报文
// Parameter: int inL 附加签名的报文总长度
// Parameter: unsigned char * outBuf 返回原始的报文
// Parameter: int & outL 输入为out缓冲区的大小，输出原始的报文的大小
// Return:    0：成功 其它：失败
//************************************
int CEccPublicKey::ParseSignature(unsigned char * inBuf, int inL, int dsIndex,unsigned char * outBuf, int & outL) //解析签名
{
	using namespace std;

	// 参数检查
	if (!ValidKey())
	{
		ostringstream ostr;
		ostr<<"公有密钥未能装载"<<endl;
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

	if (dsIndex < 0 || dsIndex >= inL)
	{
		ostringstream ostr;
		ostr<<"数字签名的报文位置非法"<<endl;
		cerr<<ostr.str();
		return -1;
	}

	// 检查签名的报文的完整性
	int count = dsIndex;
	if (inBuf[count++] != DI_SIGN_START_TAG) //检查开始标志
	{
		ostringstream ostr;
		ostr<<"数字签名的开始标志非法"<<endl;
		cerr<<ostr.str();
		return -1;
	}

	unsigned char dsL = inBuf[count++];
	unsigned char dsLIndex = count - 1;
	inBuf[count - 1] = 0;
	if(dsIndex + dsL + 2 > inL) // 检查长度
	{
		ostringstream ostr;
		ostr<<"数字签名的长度非法"<<endl;
		cerr<<ostr.str();
		return -1;
	}

	// 检查控制标志，暂略
	unsigned char Contrl_C = inBuf[count++];
	unsigned char Contrl_S = inBuf[count++];

	//检查时间戳
	time_t send_time = ntohl(BufToVal(&inBuf[count],4));
	count += 4;

	// 检查重放攻击(如果系统改时间了怎么办？)
	time_t now = time(NULL);
	if ((send_time < last_send_time_)||((now-last_local_time_) - (send_time-last_send_time_) > vaild_time_diff))
	{
		ostringstream ostr;
		ostr<<"数字签名的时间戳非法"<<"send_time = "<<send_time<<" last_send_time = "<<last_send_time_<<" now = "<<now<<" last_local_time = "<<last_local_time_<<endl;
		cerr<<ostr.str();
		return -1;
	}
	last_send_time_ = send_time;
	last_local_time_ = now;

	// 检查安全标签，暂略
	count += 16;

	// 验签
	MD5_CTX md5;
	unsigned char hashed[16];
	MD5_Init(&md5);
	MD5_Update(&md5,inBuf,count);
	MD5_Final(hashed,&md5);

	cout<<"Pub hash buf:";
	for (int i=0;i<count;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)inBuf[i];
	}
	cout<<endl;

	cout<<"Pub hash val:";
	for (int i=0;i<16;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)hashed[i];
	}
	cout<<endl;

	int sing_len = (dsL + 2) - (count - dsIndex);
	cout<<"Pub sign val:";
	for (int i=0;i<sing_len;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)inBuf[count + i];
	}
	cout<<endl;

	int ret = Authentic(hashed,sizeof(hashed),&inBuf[count],sing_len);
	if (ret) 
	{
		ostringstream ostr;
		ostr<<"Authentic fail"<<endl;
		cerr<<ostr.str();

		return -1;
	}

	if (inBuf != outBuf)
	{
		memcpy(outBuf,inBuf,dsIndex);
	}

	outL = dsIndex;
	
	ostringstream ostr;
	ostr<<"Authentic OK!"<<endl;
	cerr<<ostr.str();

	inBuf[dsLIndex] = dsL;

	return 0;
}

//************************************
// Method:    CalcSecretDataLength
// FullName:  DigitalSignature::CPublicKey::CalcSecretDataLength
// Access:    virtual public 
// Returns:   int
// Qualifier:
// Parameter: unsigned char * buf
//************************************
int CEccPublicKey::CalcSecretDataLength(unsigned char * buf,bool bCalcByFrame)
{
	size_t count = 0;

	count += HEAD_FRAME_LENGTH;
	count += buf[1];

	return count;
}

};//namespace DigitalSignature 
