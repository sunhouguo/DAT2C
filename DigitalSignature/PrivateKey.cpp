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
#include "PrivateKey.h"
#include "../PublicSupport/Dat2cPublicAPI.h"

namespace DigitalSignature {

const unsigned char DI_SIGN_START_TAG = 0x16;

CPrivateKey::CPrivateKey(void)
{
}

CPrivateKey::~CPrivateKey(void)
{
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
int CPrivateKey::AssembleSignature(const unsigned char * inBuf, int inL, unsigned char * outBuf, int & outL)
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

}; //namespace DigitalSignature

