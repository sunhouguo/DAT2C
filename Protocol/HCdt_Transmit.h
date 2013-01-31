#pragma once
#include "protocol.h"

namespace Protocol {

class CHCdt_Transmit :
	public CProtocol
{
public:
	CHCdt_Transmit(boost::asio::io_service & io_service);
	virtual ~CHCdt_Transmit(void);

	virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes);
	virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes);
	virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes);                  //返回通讯节点在容器中的序号

	virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd);

	virtual int LoadXmlCfg(std::string filename);
	virtual void SaveXmlCfg(std::string filename);

private:
	void InitFrameLocation(size_t frameHead);

private:
	unsigned short ControlByteLocation_;
	unsigned short FrameTypeLocation_;
	unsigned short InfoNumLocation_;
	unsigned short SrcAddrLocation_;
	unsigned short DstAddrLocation_;
	unsigned short ControlCrcLocation_;
};

};//namespace Protocol

