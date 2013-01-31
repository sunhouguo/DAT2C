#include <sstream>
#include <cstring>

#if defined(__linux__)
#include <netinet/in.h>
#endif

#if defined(_WIN32)
#include <winsock2.h>
#pragma comment(lib,"sm2.lib")
#endif

#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../FileSystem/Markup.h"
#include "../Protocol/Protocol.h"
#include "../DataBase/CommPoint.h"
#include "Sm2PublicKey.h"
#include "EvpKey.h"

namespace DigitalSignature {

const std::string defaultID = "ABCDEFGHIJKL";

const std::string strSM2ROOT = "Sm2";
const std::string strRandomNum = "RandomNum";
const std::string strBigNumX = "BigNumX";
const std::string strBigNumY = "BigNumY";

const unsigned char bn_x_line_index = 2;
const unsigned char bn_y_line_index = 3;
const size_t DI_SIGN_LENGTH = ECCref_MAX_LEN * 2;

const unsigned char DI_SIGN_START_TAG = 0x16;
const unsigned char HEAD_FRAME_LENGTH = 4;
const unsigned char C_FRAME_LENGTH = 1;
const unsigned char S_FRAME_LENGTH = 1;
const unsigned char ADDR_FRAME_LENGTH = 4;
const unsigned char TIME_FRAME_LENGTH = 4;
const unsigned char TAIL_FRAME_LENGTH = 2;

const unsigned char CMD_CONTROL = 1;
const unsigned char CMD_NEGOTIATE = 2;
const unsigned char CMD_CHECK_PUBKEY = 3;
const unsigned char CMD_LOAD_PUBKEY = 4;

const unsigned char SIGN_LEN = 0x4a;

using namespace std;

//************************************
// Method:    CSm2PublicKey
// FullName:  DigitalSignature::CSm2PublicKey::CSm2PublicKey
// Access:    public 
// Returns:   
// Qualifier: 构造函数
// Parameter: std::string keyPath
//************************************
CSm2PublicKey::CSm2PublicKey(std::string keyPath,Protocol::CProtocol & pl)
							:pl_(pl)
{
	keyPath_ = "";
	validKey_ = false;
	evpKey_.reset(new CEvpKey());

	sm2Key_.bits = ECCref_MAX_BITS;

	id_ = defaultID;

	LoadKey(keyPath);
}


//************************************
// Method:    ~CSm2PublicKey
// FullName:  DigitalSignature::CSm2PublicKey::~CSm2PublicKey
// Access:    virtual public 
// Returns:   
// Qualifier: 析构函数
// Parameter: void
//************************************
CSm2PublicKey::~CSm2PublicKey(void)
{
	SaveKey();

	FreeKey();
}

//************************************
// Method:    getKeyLength
// FullName:  DigitalSignature::CSm2PublicKey::getKeyLength
// Access:    virtual public 
// Returns:   int
// Qualifier: 获取密钥的长度，定长字节
//************************************
int CSm2PublicKey::getKeyLength()
{
	return DI_SIGN_LENGTH;
}

//************************************
// Method:    SaveKey
// FullName:  DigitalSignature::CSm2PublicKey::SaveKey
// Access:    private 
// Returns:   void
// Qualifier: 将公钥内容保存为文件
// Parameter: std::string keyPath 文件路径名
//************************************
void CSm2PublicKey::SaveKey()
{
	if (!keyPath_.empty())
	{
		SaveXmlCfg(keyPath_);
	}
}

//************************************
// Method:    SaveXmlCfg
// FullName:  DigitalSignature::CSm2PublicKey::SaveXmlCfg
// Access:    private 
// Returns:   void
// Qualifier: 以XML格式保存公钥数据
// Parameter: std::string filename
//************************************
void CSm2PublicKey::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	//xml.SetDoc(strSubXsl);

	xml.AddElem(strSM2ROOT);
	xml.IntoElem();//enter strSM2ROOT

	xml.AddElem(strRandomNum,TransHexToStr(pucRandom_,random_num));
	xml.AddElem(strBigNumX,TransHexToStr(sm2Key_.x,ECCref_MAX_LEN));
	xml.AddElem(strBigNumY,TransHexToStr(sm2Key_.y,ECCref_MAX_LEN));

	xml.OutOfElem();//out strSM2ROOT

	xml.Save(filename);//save xml file
}

//************************************
// Method:    LoadXmlCfg
// FullName:  DigitalSignature::CSm2PublicKey::LoadXmlCfg
// Access:    private 
// Returns:   int
// Qualifier: 读取XML格式的公钥数据
// Parameter: std::string filename
//************************************
int CSm2PublicKey::LoadXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{
		//std::cerr<<"导入xml密钥文件"<<filename<<"失败！"<<std::endl;
		return -1;
	}

	xml.ResetMainPos();
	xml.FindElem(); //root strSM2ROOT
	xml.IntoElem();  //enter strSM2ROOT

	xml.ResetMainPos();
	if (xml.FindElem(strRandomNum))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if(TransStrToHex(strTmp,pucRandom_,random_num) != random_num)
		{
			setRandom();
		}
	}
	else
	{
		setRandom();
	}

	std::string strX;
	xml.ResetMainPos();
	if (xml.FindElem(strBigNumX))
	{
		strX = xml.GetData();
		boost::algorithm::trim(strX);
	}

	std::string strY;
	xml.ResetMainPos();
	if (xml.FindElem(strBigNumY))
	{
		strY = xml.GetData();
		boost::algorithm::trim(strY);
	}

	xml.OutOfElem(); //out strSubStation

	return InitBnXY(strX,strY);
}

//************************************
// Method:    LoadBinCfg
// FullName:  DigitalSignature::CSm2PublicKey::LoadBinCfg
// Access:    private 
// Returns:   int
// Qualifier: 读取二进制格式的公钥数据
// Parameter: std::string filename
//************************************
int CSm2PublicKey::LoadBinCfg(std::string filename)
{
	ifstream infs(filename.c_str(),ios::in | ios::binary);

	if(infs.is_open())
	{
		infs.read((char *)&sm2Key_,sizeof(sm2Key_));
		sm2Key_.bits = ntohl(sm2Key_.bits);

		if(infs.gcount() == sizeof(sm2Key_))
		{
			setRandom();

			infs.close();
			infs.clear();

			return 0;
		}
	}

	infs.close();
	infs.clear();

	return -1;
}

//************************************
// Method:    LoadTxtCfg
// FullName:  DigitalSignature::CSm2PublicKey::LoadTxtCfg
// Access:    private 
// Returns:   int
// Qualifier: 读取文本格式的公钥数据
// Parameter: std::string filename
//************************************
int CSm2PublicKey::LoadTxtCfg(std::string filename)
{
	setRandom();

	return InitBnXY(getFileStr(filename,bn_x_line_index),getFileStr(filename,bn_y_line_index));
}

//************************************
// Method:    LoadKey
// FullName:  CSm2PublicKey::LoadKey
// Access:    private 
// Returns:   int
// Qualifier: 导入公钥
// Parameter: std::string keypath 公钥的大数输入
// Return:    0：成功 其它：失败
//************************************
int CSm2PublicKey::LoadKey( std::string keyPath )
{
	int ret = LoadXmlCfg(keyPath);
	if (ret < 0)
	{
		ret = LoadBinCfg(keyPath);
		if (ret < 0)
		{
			ret = LoadTxtCfg(keyPath);
		}
	}

	if (ret < 0)
	{
		return ret;
	}

	keyPath_ = keyPath;

	validKey_ = true;

	sm2_init();

	return 0;
}

//************************************
// Method:    FreeKey
// FullName:  CSm2PublicKey::FreeKey
// Access:    private 
// Returns:   int
// Qualifier: 释放内存
// Return:    0：成功 其它：失败
//************************************
int CSm2PublicKey::FreeKey()
{
	sm2_final();

	return 0;
}

//************************************
// Method:    ValidKey
// FullName:  DigitalSignature::CSm2PublicKey::ValidKey
// Access:    virtual public 
// Returns:   bool
// Qualifier: 校验公钥合法性
//************************************
bool CSm2PublicKey::ValidKey()
{
	return validKey_;
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
int CSm2PublicKey::Authentic(unsigned char *dgst, int dgstlen,unsigned char *sig, int siglen)
{
	ECCSignature stSig;
	int s_length = siglen - ECCref_MAX_LEN;
	if (s_length <= 0)
	{
		memcpy(stSig.r,sig,siglen);
	}
	else
	{
		memcpy(stSig.r,sig,ECCref_MAX_LEN);
		memcpy(stSig.s,sig + ECCref_MAX_LEN,s_length < ECCref_MAX_LEN ? s_length : ECCref_MAX_LEN);
	}

	cout<<"Ori Frame: ";
	for (int i=0;i<dgstlen;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)dgst[i];
	}
	cout<<endl;

	cout<<"Sig r: ";
	for (int i=0;i<ECCref_MAX_LEN;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)stSig.r[i];
	}
	cout<<endl;

	cout<<"Sig s: ";
	for (int i=0;i<ECCref_MAX_LEN;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)stSig.s[i];
	}
	cout<<endl;

	cout<<"Pub key bits: "<<sm2Key_.bits<<endl;

	cout<<"bn x: ";
	for (int i=0;i<ECCref_MAX_LEN;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)sm2Key_.x[i];
	}
	cout<<endl;


	cout<<"bn y: ";
	for (int i=0;i<ECCref_MAX_LEN;i++)
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)sm2Key_.y[i];
	}
	cout<<endl;

	boost::scoped_array<unsigned char> temp_buf(new unsigned char[id_.length() + 1]);
	memcpy(temp_buf.get(),id_.c_str(),id_.length());

	cout<<"ID: ";
	for (size_t i=0;i<id_.length();i++)
	{
		cout<<temp_buf[i];
	}
	cout<<endl;

	int ret =  SM2_Verify(dgst,dgstlen,temp_buf.get(),id_.length(),&sm2Key_,&stSig);
	if (ret != 0)
	{
		ostringstream ostr;
		ostr<<"SM2_Verify Fail"<<endl;
		cerr<<ostr.str();

		return ret;
	}

	ostringstream ostr;
	ostr<<"SM2_Verify OK!"<<endl;
	cout<<ostr.str();

	return 0;
}

//************************************
// Method:    ParseSignature
// FullName:  DigitalSignature::CSm2PublicKey::ParseSignature
// Access:    public 
// Return:    int 0：成功 其它：失败
// Qualifier: /*解析签名 */
// Parameter: const unsigned char * inBuf 附加签名的报文
// Parameter: int inL 附加签名的报文总长度
// Parameter: unsigned char * outBuf 返回原始的报文
// Parameter: int & outL 输入为out缓冲区的大小，输出原始的报文的大小
//************************************
int CSm2PublicKey::ParseSignature(unsigned char * inBuf, int inL, int dsIndex,unsigned char * outBuf, int & outL) //解析签名
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

	/*检查签名的报文的完整性*/

	//检查开始标志
	if (!(inBuf[dsIndex] == DI_SIGN_START_TAG && inBuf[dsIndex + 3] == DI_SIGN_START_TAG))
	{
		ostringstream ostr;
		ostr<<"数字签名的开始标志非法"<<endl;
		cerr<<ostr.str();
		return -1;
	}

	//获取安全数据区和数字签名的长度
	unsigned char dsLength = inBuf[dsIndex + 2];//签名长度
	unsigned char secureLength = inBuf[dsIndex + 1] - dsLength;//除了签名以外的其他安全数据长度
	if (secureLength <= 0 || dsLength < 0 || dsIndex + HEAD_FRAME_LENGTH + secureLength + dsLength + TAIL_FRAME_LENGTH > inL)
	{
		ostringstream ostr;
		ostr<<"安全数据区的长度非法"<<endl;
		cerr<<ostr.str();
		return -1;
	}

	//验证校验和
	unsigned char calc_sum = CalcCheckSumByte(&inBuf[dsIndex + HEAD_FRAME_LENGTH],secureLength + dsLength);
	unsigned char recv_sum = inBuf[dsIndex + HEAD_FRAME_LENGTH + secureLength + dsLength];
	if (calc_sum != recv_sum)
	{
		ostringstream ostr;
		ostr<<"安全数据区的校验和非法 ";
		ostr<<"recv_sum = "<<(short)recv_sum<<" calc_sum = "<<(short)calc_sum<<endl;
		cerr<<ostr.str();
		return -1;
	}

	//验证报文结束符
	unsigned char end_tag = inBuf[dsIndex + HEAD_FRAME_LENGTH + secureLength + dsLength + 1];
	if (end_tag != DI_SIGN_START_TAG)
	{
		ostringstream ostr;
		ostr<<"安全报文的结束符非法 end_tag = "<<end_tag<<endl;
		cerr<<ostr.str();
		return -1;
	}

	//对称密钥解密
	boost::scoped_array<unsigned char> decryptBuf(new unsigned char[inL + 1]);
	int out_len = evpKey_->decrypt(&inBuf[dsIndex + HEAD_FRAME_LENGTH],secureLength,(&(decryptBuf[dsIndex + HEAD_FRAME_LENGTH])));
	if (out_len <= 0)
	{
		ostringstream ostr;
		ostr<<"对称解密失败"<<endl;
		cerr<<ostr.str();
		return -1;
	}

	//分析报文类型
	unsigned char count = dsIndex + HEAD_FRAME_LENGTH;
	unsigned char Byte_funCode_C = decryptBuf[count++];
	unsigned char Byte_statusCode_S = decryptBuf[count++];
	unsigned char funCode_C = (Byte_funCode_C >> 2) & 0x0f;
	unsigned char keyIndex = Byte_funCode_C & 0x03;
	setKeyIndex(keyIndex);

	//检查地址
	typeAddr addr = BufToVal(&decryptBuf[count],ADDR_FRAME_LENGTH);
	share_commpoint_ptr pritstationPtr = (pl_.getCommPoint(pl_.getCommPointIndexByAddrCommType(PRISTATION_NODE,addr))).lock();
	if (!pritstationPtr)
	{
		//ostringstream ostr;
		//ostr<<"地址解析失败"<<endl;
		//cerr<<ostr.str();

		//AddNegativeCon(funCode_C,pritstationPtr);

		//return -1;

		pritstationPtr = pl_.getFirstCommPoint();
	}
	setAddr(addr);
	count += ADDR_FRAME_LENGTH;

	//检查时间戳
	time_t send_time = ntohl(BufToVal(&decryptBuf[count],4));
	count += 4;
	time_t now = time(NULL);//检查重放攻击(如果系统改时间了怎么办？)
	if ((send_time < last_send_time_)||((now-last_local_time_) - (send_time-last_send_time_) > vaild_time_diff))
	{
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)decryptBuf[count - 4]<<" ";
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)decryptBuf[count - 3]<<" ";
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)decryptBuf[count - 2]<<" ";
		cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)decryptBuf[count - 1]<<" ";
		cout<<endl;

		ostringstream ostr;
		ostr<<"数字签名的时间戳非法"<<"send_time = "<<send_time<<" last_send_time = "<<last_send_time_<<" now = "<<now<<" last_local_time = "<<last_local_time_<<endl;
		cerr<<ostr.str();

		//AddNegativeCon(funCode_C,pritstationPtr);

		//return -1;
	}
	last_send_time_ = send_time;
	last_local_time_ = now;

	//验证签名
	if (dsLength > 0)
	{
		memcpy(decryptBuf.get(),inBuf,dsIndex + HEAD_FRAME_LENGTH);
		int ret = Authentic(decryptBuf.get(),dsIndex + HEAD_FRAME_LENGTH + secureLength,&inBuf[dsIndex + HEAD_FRAME_LENGTH + secureLength],dsLength);
		//int ret = -1;
		//if(dsIndex == 0)//没有原始报文
		//{
		//	ret = Authentic(&decryptBuf[dsIndex + HEAD_FRAME_LENGTH],dsIndex + secureLength,&inBuf[dsIndex + HEAD_FRAME_LENGTH + secureLength],dsLength);
		//}
		//else //有原始报文
		//{
		//	ret = Authentic(decryptBuf.get(),dsIndex + HEAD_FRAME_LENGTH + secureLength,&inBuf[dsIndex + HEAD_FRAME_LENGTH + secureLength],dsLength);
		//}
		
		if (ret) 
		{
			ostringstream ostr;
			ostr<<"Authentic fail"<<endl;
			cerr<<ostr.str();

			AddNegativeCon(funCode_C,pritstationPtr);

			return -1;
		}
	}

	//分析报文功能号
	int ret = -1;
	switch (funCode_C)
	{
	case CMD_CONTROL:
		{
			ret = 0;

			ostringstream ostr;
			ostr<<"CMD_CONTROL CMD"<<endl;
			cout<<ostr.str();
		}
		break;

	case CMD_NEGOTIATE:
		{
			ret = 0;

			ostringstream ostr;
			ostr<<"CMD_NEGOTIATE CMD"<<endl;
			cout<<ostr.str();
		}
		break;

	case CMD_CHECK_PUBKEY:
		{
			ret = setRandom(&decryptBuf[count]);
			count += random_num;
			pl_.AddSendCmdVal(Protocol::CHECK_PUB_KEY,Protocol::CHECK_PUB_KEY_PRIORITY,pritstationPtr,true);

			ostringstream ostr;
			ostr<<"CMD_CHECK_PUBKEY CMD"<<endl;
			cout<<ostr.str();
		}
		break;

	case CMD_LOAD_PUBKEY:
		{
			setRandom(&decryptBuf[count]);
			count += random_num;
			setSm2KeyBits(BufToVal(&decryptBuf[count],sizeof(sm2Key_.bits)));
			count += sizeof(sm2Key_.bits);
			ret = setBnXY(&decryptBuf[count],ECCref_MAX_LEN,&decryptBuf[count + ECCref_MAX_LEN],ECCref_MAX_LEN,true);
			count += ECCref_MAX_LEN * 2;
			pl_.AddSendCmdVal(Protocol::UPDATE_PUB_KEY,Protocol::UPDATE_PUB_KEY_PRIORITY,pritstationPtr,true);

			ostringstream ostr;
			ostr<<"CMD_LOAD_PUBKEY CMD"<<endl;
			cout<<ostr.str();
		}
		break;

	default:
		{
			ostringstream ostr;
			ostr<<"undefined funCode_C"<<endl;
			cerr<<ostr.str();

			ret = -1;
		}
		break;
	}

	ostringstream ostr;
	ostr<<"ParseSignature Over "<<endl;
	cout<<ostr.str();

	return ret;
}

//************************************
// Method:    AddNegativeCon
// FullName:  DigitalSignature::CSm2PublicKey::AddNegativeCon
// Access:    private 
// Returns:   void
// Qualifier: 向通讯规约添加否定认可回应命令
// Parameter: unsigned char funCode_C
// Parameter: share_commpoint_ptr pritstationPtr
//************************************
void CSm2PublicKey::AddNegativeCon(unsigned char funCode_C,share_commpoint_ptr pritstationPtr)
{
	switch(funCode_C)
	{
	case CMD_CHECK_PUBKEY:
		pl_.AddSendCmdVal(Protocol::CHECK_PUB_KEY,Protocol::CHECK_PUB_KEY_PRIORITY,pritstationPtr,false);
		break;

	case CMD_LOAD_PUBKEY:
		pl_.AddSendCmdVal(Protocol::UPDATE_PUB_KEY,Protocol::UPDATE_PUB_KEY_PRIORITY,pritstationPtr,false);
		break;

	default:
		break;
	}
}

//************************************
// Method:    setBnXY
// FullName:  DigitalSignature::CSm2PublicKey::setBnXY
// Access:    public 
// Returns:   int
// Qualifier: 设置公钥大数X、Y
// Parameter: const unsigned char * bn_x
// Parameter: int xL
// Parameter: const unsigned char * bn_y
// Parameter: int yL
// Parameter: bool bSave
//************************************
int CSm2PublicKey::setBnXY(const unsigned char * bn_x,int xL,const unsigned char * bn_y,int yL,bool bSave /*=false*/)
{
	if(!(xL == ECCref_MAX_LEN && yL == ECCref_MAX_LEN))
	{
		return -1;
	}

	int retX = setBigNumX(bn_x,xL);
	int retY = setBigNumY(bn_y,yL);

	if (retX || retY)
	{
		validKey_ = false;

		return -1;
	}

	if (bSave)
	{
		SaveKey();
	}

	return 0;
}

//************************************
// Method:    InitBnXY
// FullName:  DigitalSignature::CSm2PublicKey::InitBnXY
// Access:    private 
// Returns:   int
// Qualifier: 初始化公钥大数X、Y
// Parameter: std::string BigNumX
// Parameter: std::string BigNumY
//************************************
int CSm2PublicKey::InitBnXY(std::string BigNumX,std::string BigNumY)
{
	boost::scoped_array<unsigned char> big_num_x(new unsigned char[ECCref_MAX_LEN + 1]);
	boost::scoped_array<unsigned char> big_num_y(new unsigned char[ECCref_MAX_LEN + 1]);

	int xL = TransStrToHex(BigNumX,big_num_x.get(),ECCref_MAX_LEN);
	int yL = TransStrToHex(BigNumY,big_num_y.get(),ECCref_MAX_LEN);

	//for (int i = 0;i<xL;i++)
	//{
	//	std::cout<<std::hex<<(short)sm2Key_.x[i];
	//}
	//std::cout<<std::endl;

	//for (int i=0;i<yL;i++)
	//{
	//	std::cout<<std::hex<<(short)sm2Key_.y[i];
	//}
	//std::cout<<std::endl;

	return setBnXY(big_num_x.get(),xL,big_num_y.get(),yL);
}

//************************************
// Method:    setBigNumX
// FullName:  DigitalSignature::CSm2PublicKey::setBigNumX
// Access:    private 
// Returns:   int
// Qualifier: 设置公钥大数X
// Parameter: const unsigned char * src
// Parameter: int length
//************************************
int CSm2PublicKey::setBigNumX(const unsigned char * src,int length)
{
	if (length <= 0)
	{
		return -1;
	}

	if (length > ECCref_MAX_LEN)
	{
		length = ECCref_MAX_LEN;
	}

	memcpy(sm2Key_.x,src,length);

	return 0;
}

//************************************
// Method:    setBigNumY
// FullName:  DigitalSignature::CSm2PublicKey::setBigNumY
// Access:    private 
// Returns:   int
// Qualifier: 设置公钥大数Y
// Parameter: const unsigned char * src
// Parameter: int length
//************************************
int CSm2PublicKey::setBigNumY(const unsigned char * src,int length)
{
	if (length <= 0)
	{
		return -1;
	}

	if (length > ECCref_MAX_LEN)
	{
		length = ECCref_MAX_LEN;
	}

	memcpy(sm2Key_.y,src,length);

	return 0;
}

//************************************
// Method:    CalcSecretDataLength
// FullName:  DigitalSignature::CPublicKey::CalcSecretDataLength
// Access:    virtual public 
// Returns:   int
// Qualifier: 计算安全规范标准中定义的安全数据区的长度
// Parameter: unsigned char * buf
//************************************
int CSm2PublicKey::CalcSecretDataLength(unsigned char * buf,bool bCalcByFrame)
{
	size_t count = 0;

	if (bCalcByFrame)
	{
		count += HEAD_FRAME_LENGTH;
		count += buf[1];
		count += TAIL_FRAME_LENGTH;
	}
	else
	{
		count += HEAD_FRAME_LENGTH;
		count += SIGN_LEN;
		count += TAIL_FRAME_LENGTH;
	}
	

	return count;
}

//************************************
// Method:    setRandom
// FullName:  DigitalSignature::CSm2PublicKey::setRandom
// Access:    private 
// Returns:   int
// Qualifier: 设置主站下发用的随机数
// Parameter: unsigned char[] val
//************************************
int CSm2PublicKey::setRandom(unsigned char * val)
{
	memcpy(pucRandom_,val,random_num);

	return 0;
}

//************************************
// Method:    setRandom
// FullName:  DigitalSignature::CSm2PublicKey::setRandom
// Access:    private 
// Returns:   int
// Qualifier: 初始化随机数
//************************************
int CSm2PublicKey::setRandom()
{
	return SM2_GenerateRandom(random_length,pucRandom_);
}

//************************************
// Method:    AssembleCheckKeyCon
// FullName:  DigitalSignature::CSm2PublicKey::AssembleCheckKeyCon
// Access:    public 
// Returns:   int
// Qualifier: 组装检查公钥回应报文
// Parameter: unsigned char * buf
//************************************
int CSm2PublicKey::AssembleCheckKeyCon( unsigned char * dstBuf, bool bConAct )
{
	size_t startIndex = 0;
	size_t count = startIndex;

	dstBuf[count++] = DI_SIGN_START_TAG;
	dstBuf[count++] = 0;
	dstBuf[count++] = 0;
	dstBuf[count++] = DI_SIGN_START_TAG;
	unsigned char index = getKeyIndex();
	unsigned char funCode_C = CMD_CHECK_PUBKEY;
	if (bConAct)                                                       //C
	{
		dstBuf[count++] = ((funCode_C << 2) | (index & 0x03)) & BYTE_CHECK_FALSE[7];
	}
	else
	{
		dstBuf[count++] = ((funCode_C << 2) | (index & 0x03)) | BYTE_CHECK_TRUE[7];
	}
	dstBuf[count++] = 0;                                               //S
	count += ValToBuf(&dstBuf[count],getAddr(),ADDR_FRAME_LENGTH);     // 地址
	count += ValToBuf(&dstBuf[count],htonl((u_long)time(NULL)),4);     // 时间戳
	dstBuf[count++] = 1;                                               //公钥个数
	count += ValToBuf(&dstBuf[count],0,3);                             //保留3字节

	int encryptLength = encrypt(pucRandom_,random_num,&dstBuf[count]); //公钥加密
	if (encryptLength <= 0)
	{
		return -1;
	}
	count += encryptLength;

	int ret = evpKey_->encrypt(&dstBuf[startIndex + HEAD_FRAME_LENGTH],count - encryptLength,&dstBuf[startIndex + HEAD_FRAME_LENGTH]); //对称加密
	if (ret != (count - encryptLength))
	{
		return -1;
	}

	dstBuf[1] = count - HEAD_FRAME_LENGTH;                             //安全数据区的长度，包括公钥加密的数据，不包括报文头
	dstBuf[2] = encryptLength;                                         //公钥加密数据的长度

	dstBuf[count++] = CalcCheckSumByte(&dstBuf[startIndex + HEAD_FRAME_LENGTH],count);
	dstBuf[count++] = DI_SIGN_START_TAG;

	return count - startIndex;
}

//************************************
// Method:    AssembleUpdateKeyCon
// FullName:  DigitalSignature::CSm2PublicKey::AssembleUpdateKeyCon
// Access:    public 
// Returns:   int
// Qualifier: 检查更新公钥回应报文
// Parameter: unsigned char * buf
//************************************
int CSm2PublicKey::AssembleUpdateKeyCon( unsigned char * dstBuf, bool bConAct )
{
	size_t startIndex = 0;
	size_t count = startIndex;

	dstBuf[count++] = DI_SIGN_START_TAG;
	dstBuf[count++] = 0;
	dstBuf[count++] = 0;
	dstBuf[count++] = DI_SIGN_START_TAG;
	unsigned char index = getKeyIndex();
	unsigned char funCode_C = CMD_LOAD_PUBKEY;
	if (bConAct)                                                       //C
	{
		dstBuf[count++] = ((funCode_C << 2) | (index & 0x03)) & BYTE_CHECK_FALSE[7];
	}
	else
	{
		dstBuf[count++] = ((funCode_C << 2) | (index & 0x03)) | BYTE_CHECK_TRUE[7];
	}
	dstBuf[count++] = 0;                                               //S
	count += ValToBuf(&dstBuf[count],getAddr(),ADDR_FRAME_LENGTH);     // 地址
	count += ValToBuf(&dstBuf[count],htonl((u_long)time(NULL)),4);     // 时间戳
	
	int encryptLength = encrypt(pucRandom_,random_num,&dstBuf[count]); //公钥加密
	if (encryptLength <= 0)
	{
		return -1;
	}
	count += encryptLength;

	int ret = evpKey_->encrypt(&dstBuf[startIndex + HEAD_FRAME_LENGTH],count - encryptLength,&dstBuf[startIndex + HEAD_FRAME_LENGTH]); //对称加密
	if (ret != (count - encryptLength))
	{
		return -1;
	}

	dstBuf[1] = count - HEAD_FRAME_LENGTH;                             //安全数据区的长度，包括公钥加密的数据,不包括报文头
	dstBuf[2] = encryptLength;                                         //公钥加密数据的长度

	dstBuf[count++] = CalcCheckSumByte(&dstBuf[startIndex + HEAD_FRAME_LENGTH],count);
	dstBuf[count++] = DI_SIGN_START_TAG;

	return count - startIndex;
}

//************************************
// Method:    encrypt
// FullName:  DigitalSignature::CSm2PublicKey::encrypt
// Access:    private 
// Returns:   int
// Qualifier: 公钥加密
// Parameter: const unsigned char * src
// Parameter: int srcLength
// Parameter: unsigned char * dst
//************************************
int CSm2PublicKey::encrypt(unsigned char * src,int srcLength,unsigned char * dst)
{
	boost::scoped_array<unsigned char> random_buf(new unsigned char[ECCref_MAX_LEN + 1]);
	memset(random_buf.get(),0,ECCref_MAX_LEN + 1);

	if(SM2_GenerateRandom((ECCref_MAX_BITS - 1),random_buf.get()) == 0)
	{
		cout<<"Ori Frame:";
		for (int i=0;i<srcLength;i++)
		{
			cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)src[i];
		}
		cout<<endl;

		boost::scoped_array<unsigned char> src_buf(new unsigned char[ECCref_MAX_LEN + 1]);
		memset(random_buf.get(),0,ECCref_MAX_LEN + 1);
		if(srcLength > ECCref_MAX_LEN)
		{
			srcLength = ECCref_MAX_LEN;
		}
		memcpy(src_buf.get(),src,srcLength);

		ECCCipher cipher;
		if(SM2_Encrypt(src_buf.get(),ECCref_MAX_LEN,&sm2Key_,random_buf.get(),ECCref_MAX_BITS,&cipher) == 0)
		{
			memcpy(dst,&cipher.x,ECCref_MAX_LEN);
			memcpy(dst + ECCref_MAX_LEN,&cipher.y,ECCref_MAX_LEN);
			memcpy(dst + (ECCref_MAX_LEN * 2),&cipher.C,ECCref_MAX_LEN);
			memcpy(dst + (ECCref_MAX_LEN * 3),&cipher.M,ECCref_MAX_LEN);

			cout<<"Ent Frame x:";
			for(int i=0;i<ECCref_MAX_LEN;i++)
			{
				cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)cipher.x[i];
			}
			cout<<endl;

			cout<<"Ent Frame y:";
			for(int i=0;i<ECCref_MAX_LEN;i++)
			{
				cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)cipher.y[i];
			}
			cout<<endl;

			cout<<"Ent Frame C:";
			for(int i=0;i<ECCref_MAX_LEN;i++)
			{
				cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)cipher.C[i];
			}
			cout<<endl;

			cout<<"Ent Frame M:";
			for(int i=0;i<ECCref_MAX_LEN;i++)
			{
				cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)cipher.M[i];
			}
			cout<<endl;
			
			return ECCref_MAX_LEN * 4;
		}
	}

	return -1;
}

//************************************
// Method:    getAddr
// FullName:  DigitalSignature::CSm2PublicKey::getAddr
// Access:    private 
// Returns:   typeAddr
// Qualifier: 获取终端地址
//************************************
typeAddr CSm2PublicKey::getAddr()
{
	return addr_;
}

//************************************
// Method:    setAddr
// FullName:  DigitalSignature::CSm2PublicKey::setAddr
// Access:    private 
// Returns:   int
// Qualifier: 设置终端地址
// Parameter: typeAddr val
//************************************
int CSm2PublicKey::setAddr(typeAddr val)
{
	addr_ = val;

	return 0;
}

//************************************
// Method:    getKeyIndex
// FullName:  DigitalSignature::CSm2PublicKey::getKeyIndex
// Access:    private 
// Returns:   int
// Qualifier: 获取公钥序号
//************************************
int CSm2PublicKey::getKeyIndex()
{
	return keyIndex_;
}

//************************************
// Method:    setKeyIndex
// FullName:  DigitalSignature::CSm2PublicKey::setKeyIndex
// Access:    private 
// Returns:   int
// Qualifier: 设置公钥序号
// Parameter: int val
//************************************
int CSm2PublicKey::setKeyIndex(int val)
{
	keyIndex_ = val;

	return 0;
}

//************************************
// Method:    getSm2KeyBits
// FullName:  DigitalSignature::CSm2PublicKey::getSm2KeyBits
// Access:    private 
// Returns:   int
// Qualifier: 获取公钥模长
//************************************
int CSm2PublicKey::getSm2KeyBits()
{
	return sm2Key_.bits;
}

//************************************
// Method:    setSm2KeyBits
// FullName:  DigitalSignature::CSm2PublicKey::setSm2KeyBits
// Access:    private 
// Returns:   int
// Qualifier: 设置公钥模长
// Parameter: int val
//************************************
int CSm2PublicKey::setSm2KeyBits(int val)
{
	sm2Key_.bits = val;

	return 0;
}

};//namespace DigitalSignature
