#pragma once
#include "Protocol.h"

namespace FileSystem
{
class CMarkup;
}

namespace Protocol{

class CH101_B:
	public CProtocol
{
public:
	CH101_B(boost::asio::io_service & io_service);
	~CH101_B(void);

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

	virtual int QueryUnAliveCommPoint(share_commpoint_ptr point);

private:
	//Init api 
	void InitObjectIndex();
	void InitDefaultStartAddr();
	void InitDefaultFrameElem();
	void InitFrameLocation(size_t FrameHead);
	void InitDefaultTimeOut();
	void InitDefaultTimer(boost::asio::io_service & io_service);

	//timer
	void handle_timerRequireLink(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerCallAllData(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerSynTime(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerTestAct(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerCallAllDD(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);


	void ResetTimerRequireLink(share_commpoint_ptr point,bool bContinue = true, unsigned short val = 0);
	void ResetTimerCallAllData(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
	void ResetTimerSynTime(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
	void ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
	void ResetTimerTestAct(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
	void ResetTimerCallAllDD(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
	void ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYkExe(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);

    //send frame assemble
	int AssembleRequireLink(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
	int AssembleResetLink(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
	int AssembleLinkStatus(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
	int AssembleConfirmAck(size_t bufIndex,unsigned char * buf,share_terminal_ptr terminalPtr);
	int AssembleCallAllData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
	int AssembleSynTime(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr, boost::posix_time::ptime time);
	int AssembleTestAck(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
	int AssembleCallAllDD(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
	int AssembleDoubleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code);
	int AssembleDoubleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code);
	int AssembleDoubleYKCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no,unsigned char yk_code);
	int AssembleSingleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code);
	int AssembleSingleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no, unsigned char yk_code);
	int AssembleSingleYKCancel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned short yk_no,unsigned char yk_code);


	//recv frame parse
	int ParseShortFrame(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseLongFrame(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllDataCallCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllDataCallOver(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllSingleYX(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseSingleCOS(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllDoubleYX(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseDoubleCOS(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllYXByte(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllYCData(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseYCCH(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllYCDataWithValid(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseYCCHWithValid(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseSynTimeCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseSingleSOE(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseDoubleSOE(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllYMCallCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllYMCallOver(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseAllYMData(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseEndInit(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseDoubleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseDoubleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseDoubleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseDoubleYKOverCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseSingleYKSelCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseSingleYKExeCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseSingleYKCancelCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseSingleYKOverCon(unsigned char * buf, share_terminal_ptr terminalPtr);
	int ParseTestFramCon(unsigned char * buf, share_terminal_ptr terminalPtr);

	//protocol func api
	int getAddrByRecvFrame(unsigned char * buf);
	int getFCB(share_terminal_ptr terminalPtr);

private:

	//para api
	int setSYX_START_ADDR(size_t val);
	int setDYX_START_ADDR(size_t val);
	int setYC_START_ADDR(size_t val);
	int setSYK_START_ADDR(size_t val);
	int setDYK_START_ADDR(size_t val);
	int setYM_START_ADDR(size_t val);
	int setHIS_START_ADDR(size_t val);

	int setFrameTypeLength(unsigned short val);
	int setInfoNumLength(unsigned short val);
	int setTransReasonLength(unsigned short val);
	int setAsduAddrLength(unsigned short val);
	int setInfoAddrLength(unsigned short val);

	int setTimeOutQueryUnActivePoint(unsigned short val);
	int setTimeOutRequrieLink(unsigned short val);
	int setTimeOutCallAllData(unsigned short val);
	int setTimeOutCallAllDD(unsigned short val);
	int setTimeOutSynTime(unsigned short val);
	int setTimeOutHeartFrame(unsigned short val);
	int setTimeOutYkSel(unsigned short val);
	int setTimeOutYkExe(unsigned short val);
	int setTimeOutYkCancel(unsigned short val);
	int setTimeOutCallPara(unsigned short val);
	int setTimeOutSetPara(unsigned short val);

private:
	enum h101Para
	{
		DEFAULT_SYX_START_ADDR = 0x0001,                        //默认单点yx起始地址
		DEFAULT_DYX_START_ADDR = 0x0001,                        //默认双点yx起始地址
		DEFAULT_YC_START_ADDR = 0x4001,                         //默认yc起始地址
		DEFAULT_SYK_START_ADDR = 0x6001,                        //默认单点yk起始地址
		DEFAULT_DYK_START_ADDR = 0x6001,                        //默认双点yk起始地址
		DEFAULT_YM_START_ADDR = 0x6401,                         //默认ym起始地址
		DEFAULT_HIS_START_ADDR = 0x0000,                        //默认历史数据起始地址

		DEFAULT_FrameTypeLength = 1,                            //默认报文类型标识的字节长度
		DEFAULT_InfoNumLength = 1,                              //默认信息体数目标识的字节长度
		DEFAULT_TransReasonLength = 1,                          //默认传送原因标识的字节长度
		DEFAULT_AsduAddrLength = 1,                             //默认装置地址标识的字节长度
		DEFAULT_InfoAddrLength = 2,                             //默认信息体地址标识的字节长度

		DEFAULT_timeOutQueryUnActivePoint = 600,
		MIN_timeOutQueryUnActivePoint = 60,
		DEFAULT_timeOutRequireLink = 10,//3,
		MIN_timeOutRequireLink = 1,
		DEFAULT_timeOutCallAllData = 600,
		MIN_timeOutCallAllData = 60,
		DEFAULT_timeOutCallAllDD = 900,
		MIN_timeOutCallAllDD = 180,
		DEFAULT_timeOutSynTime = 1800,
		MIN_timeOutSynTime = 60,
		DEFAULT_timeOutHeartFrame = 10,//4,
		MIN_timeOutHeartFrame = 1,
		DEFAULT_timeOutYkSel = 25,
		DEFAULT_timeOutYkExe = 25,
		DEFAULT_timeOutYkCancel = 25,
		DEFAULT_timeOutCallPara = 25,
		DEFAULT_timeOutSetPara = 25,
		DEFAULT_timeOutTestAct = 30
	};

	bool bHeartTimeOutResetLink_;                    //超时未收到心跳包回复是否重启链路，默认 true

	static size_t H101_BObjectCounter_;

	size_t SYX_START_ADDR_;                              //单点yx起始地址
	size_t DYX_START_ADDR_;                              //双点yx起始地址
	size_t YC_START_ADDR_;                               //yc起始地址
	size_t SYK_START_ADDR_;                              //单点yk起始地址
	size_t DYK_START_ADDR_;                              //双点yk起始地址
	size_t YM_START_ADDR_;                               //ym起始地址
	size_t HIS_START_ADDR_;                              //历史数据起始地址

	unsigned short FrameTypeLength_;                             //报文类型标识的字节长度
	unsigned short InfoNumLength_;                               //信息体数目标识的字节长度
	unsigned short TransReasonLength_;                           //传送原因标识的字节长度
	unsigned short AsduAddrLength_;                              //装置地址标识的字节长度
	unsigned short InfoAddrLength_;                              //信息体地址标识的字节长度

	unsigned short FrameTypeLocation_;                           //报文类型标识的字节定位
	unsigned short InfoNumLocation_;                             //信息体数目标识的字节定位
	unsigned short TransReasonLocation_;                         //传送原因标识的字节定位
	unsigned short AsduAddrLocation_;                            //装置地址标识的字节定位
	unsigned short InfoAddrLocation_;                            //信息体地址标识的字节定位
	unsigned short DataLocation_;                                //数据标识的字节定位


	//timer
	unsigned short timeOutQueryUnActivePoint_;
	bool bUseTimeOutQueryUnActivePoint_;
	unsigned short timeOutRequireLink_;
	timerPtr timerRequireLink_;
	unsigned short timeOutCallAllData_;
	timerPtr timerCallAllData_;
	unsigned short timeOutCallAllDD_;
	timerPtr timerCallAllDD_;
	unsigned short timeOutSynTime_;
	timerPtr timerSynTime_;
	unsigned short timeOutHeartFrame_;
	timerPtr timerHeartFrame_;
	unsigned short timeOutYkSel_;
	timerPtr timerYkSel_;
	unsigned short timeOutYkExe_;
	timerPtr timerYkExe_;
	unsigned short timeOutYkCancel_;
	timerPtr timerYkCancel_;
	unsigned short timeOutCallPara_;
	timerPtr timerCallPara_;
	unsigned short timeOutSetPara_;
	timerPtr timerSetPara_;
	unsigned short timeOutTestAct_;
	timerPtr timerTestAct_;
};
};//namespace Protocol
