#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstring>

#if defined(__linux__)
#include <netinet/in.h>
#endif
//
//#if defined(_WIN32)
//#include <winsock2.h>
//#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib,"libeay32.lib")
//#endif

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/md5.h>

#include "../PublicSupport/Dat2cPublicAPI.h"
#include "SpeEccPublicKey.h"

namespace DigitalSignature {

const char rnd_seed[] = "string to make the random number generator think it has entropy";
const size_t DI_SIGN_LENGTH = 63;
const unsigned char DI_SIGN_START_TAG = 0x16;

using namespace std;

CSpeEccPublicKey::CSpeEccPublicKey(std::string keypath)
{
	last_send_time_ = 0;
	last_local_time_ = 0;

	key_ = NULL;

	LoadKey(keypath);
}

CSpeEccPublicKey::~CSpeEccPublicKey(void)
{
	FreeKey();
}

int CSpeEccPublicKey::getKeyLength()
{
	return DI_SIGN_LENGTH;
}

//装载公钥
//************************************
// Method:    LoadKey
// FullName:  CSpeEccPublicKey::LoadKey
// Access:    private 
// Returns:   int
// Qualifier:
// Parameter: std::string keypath ECC证书文件路径
// Return:    0：成功 其它：失败
//************************************
int CSpeEccPublicKey::LoadKey(std::string keypath)
{
	key_ = EC_KEY_new();

	BIO *bio = BIO_new_file(keypath.c_str(),"rb");
	key_ = d2i_EC_PUBKEY_bio( bio, &key_ );

	BIO_free(bio);
	return 0;
}

//************************************
// Method:    FreeKey
// FullName:  CSpeEccPublicKey::FreeKey
// Access:    private 
// Returns:   int
// Qualifier: 释放内存
// Return:    0：成功 其它：失败
//************************************
int CSpeEccPublicKey::FreeKey()
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
// Method:    ValidKey
// FullName:  DigitalSignature::CSpeEccPublicKey::ValidKey
// Access:    virtual public 
// Returns:   bool
// Qualifier:
//************************************
bool CSpeEccPublicKey::ValidKey()
{
	if (key_)
	{
		return EC_KEY_check_key(key_) == 1;
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
int CSpeEccPublicKey::Authentic(unsigned char *dgst, int dgstlen,unsigned char *sig, int siglen)
{
	RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */

	if (!key_)
	{
		ostringstream ostr;
		ostr<<"can't load key"<<endl;
		cerr<<ostr.str();

		return -1;
	}

	//std::cout<<"第三个参数dgstlen="<<dgstlen<<",第五个参数siglen="<<siglen<<std::endl;

	int ret = ECDSA_verify(0,dgst,dgstlen,sig,siglen,key_);
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
int CSpeEccPublicKey::ParseSignature(unsigned char * inBuf, int inL, int dsIndex,unsigned char * outBuf, int & outL) //解析签名
{
	// 参数检查
	if (!key_)
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
	//cout<<"现在dsIndex的值是："<<dsIndex<<endl;
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

	//std::cout<<"Spe收到变量为inL="<<inL<<",dsIndex="<<dsIndex<<std::endl;
	//cout<<"参加解密的报文:";
	//for (int i=0;i<inL;i++)
	//{
	//	cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)inBuf[i];
	//}
	//cout<<endl;

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
	time_t send_time = /*ntohl*/(BufToVal(&inBuf[count],4));
	count += 4;

	// 检查重放攻击(如果系统改时间了怎么办？)
	time_t now = time(NULL);
	//if ((send_time < last_send_time_)||((now-last_local_time_) - (send_time-last_send_time_) > vaild_time_diff))
	time_t Time;
	if (now > send_time)
	{
		Time = now - send_time;
	} 
	else
	{
		Time = send_time - now;
	}

	if (Time > 10)
	{
		ostringstream ostr;
		ostr<<"数字签名的时间戳非法Time="<<Time<<",send_time = "<<send_time<<" last_send_time = "<<last_send_time_<<" now = "<<now<<" last_local_time = "<<last_local_time_<<endl;
		cerr<<ostr.str();
		return -1;
	}
	else
	{
		ostringstream ostr;
		ostr<<"数字签名的时间戳正确Time="<<Time<<",send_time = "<<send_time<<" last_send_time = "<<last_send_time_<<" now = "<<now<<" last_local_time = "<<last_local_time_<<endl;
		cerr<<ostr.str();
	}
	last_send_time_ = send_time;
	last_local_time_ = now;

	// 检查安全标签，暂略
	count += 16;

	// 验签
	//MD5_CTX md5;
	//unsigned char hashed[16];
	//MD5_Init(&md5);
	//MD5_Update(&md5,inBuf,count);
	//MD5_Final(hashed,&md5);

	//cout<<"Pub hash buf:";
	//for (int i=0;i<count;i++)
	//{
	//	cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)inBuf[i];
	//}
	//cout<<endl;

	//cout<<"Pub hash val:";
	//for (int i=0;i<16;i++)
	//{
	//	cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)hashed[i];
	//}
	//cout<<endl;

	int sing_len = (dsL + 2) - (count - dsIndex);
	//   cout<<"The sig_len is:"<<sing_len<<endl;
	//cout<<"Pub sign val:";
	//for (int i=0;i<sing_len;i++)
	//{
	//	cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)inBuf[count + i];
	//}
	//cout<<endl;

	//int ret = Authentic(hashed,sizeof(hashed),&inBuf[count],sing_len);
	int ret = Authentic(inBuf,count,&inBuf[count],sing_len);
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

};//namespace DigitalSignature 


