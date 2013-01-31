#pragma once

#include "Protocol.h"

namespace Protocol {

class CH103_NZ_UDP
	:public CProtocol
{
public:
	CH103_NZ_UDP(boost::asio::io_service & io_service);
	virtual ~CH103_NZ_UDP(void);

	virtual int LoadXmlCfg(std::string filename);
	virtual void SaveXmlCfg(std::string filename);

protected:
	virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes);
	virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes);
	virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes);

	virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd);

	virtual int InitProtocol();
	virtual void UninitProtocol();

private:
	void InitObjectIndex();
	void InitDefaultTimeOut();
	void InitDefaultTimer(boost::asio::io_service & io_service);

	void handle_timerUDP(const boost::system::error_code& error,share_commpoint_ptr point);
	void ResetTimerUDP(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
	int setTimeOutUDP(unsigned short val);
	int AssembleUDP(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);

private:
	enum H103_NZ_UDPPara
	{
		BroadCastAddr = 0xff,

		DEFAULT_timeOutUDP = 5,
		MIN_timeOutUDP = 5,
	};

	static size_t H103_NZ_UDPObjectCounter_;

	unsigned short timeOutUDP_;
	timerPtr timerUDP_;

	bool bSendBroadCast_;
};

}; //namespace Protocol 

