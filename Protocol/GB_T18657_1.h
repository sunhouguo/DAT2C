#pragma once
#include "Protocol.h"

namespace Protocol {

class CGB_T18657_1 :
	public CProtocol
{
public:
	CGB_T18657_1(boost::asio::io_service & io_service);
	virtual ~CGB_T18657_1(void);

	virtual int LoadXmlCfg(std::string filename);
	virtual void SaveXmlCfg(std::string filename);

protected:
	virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes);
	virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes);
	virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes);                  //返回通讯节点在容器中的序号

	virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd);

	virtual int InitProtocol();
	virtual void UninitProtocol();

private:
	void InitObjectIndex();
	void InitDefaultFrameElem();
	void InitFrameLocation(size_t FrameHead);

	int AssembleCallPrimaryData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char funCode);
	int AssembleCallSecondaryData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char funCode);
	int AssembleResetCMD(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char funCode);
	int AssembleLogin(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);

	int ParseBasicYxData(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseBasicYcData(unsigned char * buf, share_terminal_ptr terminalPtr);

	void InitDefaultTimeOut();
	void InitDefaultTimer(boost::asio::io_service & io_service);

	void handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point);
	void ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);

	int InitPrimaryDataFunCodeSet();
	int InitSecondaryDataFunCodeSet();

	int getNextPrimaryDataFunCode(int comm_point_index);
	int getNextSecondaryDataFunCode(int comm_point_index);

	//protocol func api
	int getAddrByRecvFrame(unsigned char * buf);
	int getFCB(share_terminal_ptr terminalPtr);

private:
	enum
	{
		defautl_DevTypeLength = 1,
		default_AreaCodeLength = 1,
		default_TerminalAddrLength = 2,
		default_PristationAddrLength = 1,
		default_AFN_Length = 1,
		default_SEQ_Length = 1,
		default_FunCodeLength = 1,

		default_pritstation_addr = 1,

		DEFAULT_timeOutHeartFrame = 1,
		MIN_timeOutHeartFrame = 1,
	};

	static size_t GB_T18657_1ObjectCounter_;
	
	unsigned short DevTypeLength_;
	unsigned short AreaCodeLength_;
	unsigned short TerminalAddrLength_;
	unsigned short PristationAddrLength_;
	unsigned short AFN_Length_;
	unsigned short SEQ_Length_;
	unsigned short FunCodeLength_;

	unsigned short DevTypeLocation_;
	unsigned short AreaCodeLocation_;
	unsigned short TerminalAddrLocation_;
	unsigned short PristationAddrLocation_;
	unsigned short AFN_Location_;
	unsigned short SEQ_Location_;
	unsigned short FunCodeLocation_;
	unsigned short FunDataLocation_;

	unsigned short Pristation_MSA_;
	unsigned short Pristation_CON_;

	std::vector<unsigned char> PrimaryDataFunCodeSet_;
	std::vector<unsigned char> SecondaryDataFunCodeSet_;

	std::vector<size_t>PrimaryDataFunCodeIndex_;
	std::vector<size_t>SecondaryDataFunCodeIndex_;
	
	//timer
	unsigned short timeOutHeartFrame_;
	timerPtr timerHeartFrame_;
};

};//namespace Protocol

