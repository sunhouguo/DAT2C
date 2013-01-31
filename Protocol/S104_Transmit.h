#pragma once
#include "protocol.h"

namespace Protocol {

class CS104_Transmit :
	public CProtocol
{
public:
	CS104_Transmit(boost::asio::io_service & io_service);
	virtual ~CS104_Transmit(void);

	virtual int MatchFrameHead( size_t & exceptedBytes );
	virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes);
	virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes);
	virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes);                  //返回通讯节点在容器中的序号

	virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd);

	virtual int LoadXmlCfg(std::string filename);
	virtual void SaveXmlCfg(std::string filename);

	virtual int InitProtocol();
	virtual void UninitProtocol();

	void ProcessSubAliveSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);

private:
	int setFrameLenLength(unsigned short val);
	int setIGramCounterLength(unsigned short val);
	int setFrameTypeLength(unsigned short val);
	int setInfoNumLength(unsigned short val);
	int setTransReasonLength(unsigned short val);
	int setAsduAddrLength(unsigned short val);
	int setInfoAddrLength(unsigned short val);

	void InitDefaultFrameElem();
	void InitFrameLocation(size_t frameHead);

	int ParseSynTime(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseYkSign(unsigned char * buf,share_pristation_ptr pristationPtr,size_t exceptedBytes);

private:
	enum S104_TransmitPara
	{
		DEFAULT_FrameLenLength = 1,                             //默认报文长度标识的字节长度
		DEFAULT_IGramCounterLength = 2,
		DEFAULT_FrameTypeLength = 1,                            //默认报文类型标识的字节长度
		DEFAULT_InfoNumLength = 1,                              //默认信息体数目标识的字节长度
		DEFAULT_TransReasonLength = 2,                          //默认传送原因标识的字节长度
		DEFAULT_AsduAddrLength = 2,                             //默认装置地址标识的字节长度
		DEFAULT_InfoAddrLength = 3,                             //默认信息体地址标识的字节长度
	};

	unsigned short FrameLenLength_;                              //报文长度标识的字节长度
	unsigned short IGramCounterLength_;
	unsigned short FrameTypeLength_;                             //报文类型标识的字节长度
	unsigned short InfoNumLength_;                               //信息体数目标识的字节长度
	unsigned short TransReasonLength_;                           //传送原因标识的字节长度
	unsigned short AsduAddrLength_;                              //装置地址标识的字节长度
	unsigned short InfoAddrLength_;                              //信息体地址标识的字节长度

	unsigned short FrameLenLocation_;                            //报文长度标识的字节定位
	unsigned short RecvCounterLocation_;
	unsigned short SendCounterLocation_;
	unsigned short FrameTypeLocation_;                           //报文类型标识的字节定位
	unsigned short InfoNumLocation_;                             //信息体数目标识的字节定位
	unsigned short TransReasonLocation_;                         //传送原因标识的字节定位
	unsigned short AsduAddrLocation_;                            //装置地址标识的字节定位
	unsigned short InfoAddrLocation_;                            //信息体地址标识的字节定位
	unsigned short DataLocation_;                                //数据标识的字节定位
};

};//namespace Protocol
