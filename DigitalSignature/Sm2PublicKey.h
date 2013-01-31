#pragma once
#include <boost/scoped_ptr.hpp>
#include "publickey.h"
#include "sm2.h"
#include "../PublicSupport/Dat2cTypeDef.h"

namespace Protocol
{
	class CProtocol;
}

namespace DigitalSignature {

typedef unsigned int typeAddr;

class CEvpKey;

class CSm2PublicKey :
	public CPublicKey
{
public:
	CSm2PublicKey(std::string keyPath,Protocol::CProtocol & pl);
	virtual ~CSm2PublicKey(void);

	virtual bool ValidKey();
	virtual int getKeyLength();

	virtual int ParseSignature(unsigned char * inBuf, int inL, int dsIndex,unsigned char * outBuf, int & outL); //解析签名
	virtual int CalcSecretDataLength(unsigned char * buf,bool bCalcByFrame);	

	int AssembleCheckKeyCon(unsigned char * dstBuf, bool bConAct);
	int AssembleUpdateKeyCon(unsigned char * dstBuf, bool bConAct);

private:
	virtual int Authentic(unsigned char *dgst, int dgstlen,unsigned char *sig, int siglen);                     //验证

	void AddNegativeCon(unsigned char funCode_C,share_commpoint_ptr pritstationPtr);

	int encrypt(unsigned char * src,int srcLength,unsigned char * dst);                           //加密
	int FreeKey();

	int LoadKey(std::string keyPath);
	void SaveKey();

	int setBnXY(const unsigned char * bn_x,int xL,const unsigned char * bn_y,int yL,bool bSave = false);
	int InitBnXY(std::string BigNumX,std::string BigNumY);
	int setBigNumX(const unsigned char * src,int length);
	int setBigNumY(const unsigned char * src,int length);

	int LoadBinCfg(std::string filename);
	int LoadTxtCfg(std::string filename);
	int LoadXmlCfg(std::string filename);
	void SaveXmlCfg(std::string filename);

	int setRandom(unsigned char * val);
	int setRandom();

	typeAddr getAddr();
	int setAddr(typeAddr val);

	int getKeyIndex();
	int setKeyIndex(int val);
	
	int getSm2KeyBits();
	int setSm2KeyBits(int val);

private:
	enum
	{
		max_key_num = 4,
		random_num = 4,
		random_length = 8 * random_num,
	};

	boost::scoped_ptr<CEvpKey> evpKey_;      //对称加密处理类
	
	std::string keyPath_;                    //非对称密钥文件名
	ECCrefPublicKey sm2Key_;                 //非对称加密密钥
	unsigned char keyIndex_;                 //非对称密钥序号
	std::string id_;                         //非对称密钥ID
	bool validKey_;                          //非对称密钥的合法性判标志，这个sm2的库居然没有设计提供检查密钥合法性的API (╰_╯)#
	unsigned char pucRandom_[random_num + 1];//保存私钥签名端发过来的随机数

	Protocol::CProtocol & pl_;               //我也不想这样的，这个密钥检查和更新设计的体系是变态的
	typeAddr addr_;                          //终端地址
};

};//namespace DigitalSignature

