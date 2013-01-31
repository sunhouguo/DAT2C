#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

#if defined(__linux__)
#include <netinet/in.h>
#endif

#if defined(_WIN32)
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"libeay32.lib")
#endif

#include <openssl/md5.h>
#include "PublicKey.h"
#include "../PublicSupport/Dat2cPublicAPI.h"

namespace DigitalSignature {

const unsigned char DI_SIGN_START_TAG = 0x16;
const unsigned char HEAD_FRAME_LENGTH = 2;

CPublicKey::CPublicKey(void)
{
	last_send_time_ = 0;
	last_local_time_ = 0;
}

CPublicKey::~CPublicKey(void)
{
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
int CPublicKey::ParseSignature(unsigned char * inBuf, int inL, int dsIndex,unsigned char * outBuf, int & outL) //解析签名
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
int CPublicKey::CalcSecretDataLength( unsigned char * buf,bool bCalcByFrame )
{
	size_t count = 0;

	count += HEAD_FRAME_LENGTH;
	count += buf[1];

	return count;
}

}; //namespace DigitalSignature
