#pragma once
#include "Protocol.h"

namespace Protocol {

class CTestHProcotol :
	public CProtocol
{
public:
	CTestHProcotol(boost::asio::io_service & io_service);
	virtual ~CTestHProcotol(void);

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

    private:
		int AssembleStartDTAct(size_t bufIndex, unsigned char * buf);
		int AssembleTestFRAct(size_t bufIndex, unsigned char * buf);

		void InitDefaultTimer(boost::asio::io_service & io_service);
		void handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point);
		void ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);

    private:
		enum hTestPara
		{
			DEFAULT_timeOutHeartFrame = 10
		};

		static size_t HTestObjectCounter_;

		unsigned short timeOutHeartFrame_;
		timerPtr timerHeartFrame_;

		boost::posix_time::ptime begin_time_;
		unsigned short AssembleCount_;
};

}; //namespace Protocol 
