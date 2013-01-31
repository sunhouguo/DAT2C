#pragma once
#include "Protocol.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase
{
	class CCosPoint;
	class CSoePoint;
	class CYcVarPoint;
}

namespace Protocol {

typedef size_t typeInfoNumLimit;

class CS104:
	public CProtocol
{
public:
	CS104(boost::asio::io_service & io_service);
	~CS104(void);

	virtual int LoadXmlCfg(std::string filename);
	virtual void SaveXmlCfg(std::string filename);

protected:
	virtual int MatchFrameHead( size_t & exceptedBytes );
	virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes);
	virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes);
	virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes);

	virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameTail(size_t bufBegin,size_t bufIndex, unsigned char * buf, CCmd & cmd);

	virtual int InitProtocol();
	virtual void UninitProtocol();

	int ConnectSubYkSig(share_commpoint_ptr point);
	int DisconnectSubYkSig(share_commpoint_ptr point,bool bForceClose);
	virtual void ProcessSubYkSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);

protected:
	bool CheckIFrameRecvCounter(unsigned short sendVal);
	bool CheckIFrameSendCounter(unsigned short recvVal);
	void ClearIFrameCounter();

	bool CheckCounterKU();
	bool CheckCounterKI();
	bool CheckCounterWI();
	void ClearCounterKU();
	void ClearCounterKI();
	void ClearCounterWI();
	void IGramFrameSend();
	void UGramFrameSend();
	void IGramFrameRecv();

	void InitFrameLocation(size_t FrameHead);

	void ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
	void ResetTimerAnyFrameRecv(bool bContinue = true,unsigned short val = 0);
	void ResetTimerIGramFrameRecv(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);

	int ParseFrame_I(unsigned char * buf,size_t exceptedBytes);

private:
	void InitObjectIndex();

	void InitDefaultStartAddr();
	void InitDefaultFrameElem();
	void InitDefaultInfoNumLimit();
	void InitDefaultTimeOut();
	void InitDefaultTimer(boost::asio::io_service & io_service);
	void InitFrameLength();
	
	//para api
	int setSYX_START_ADDR(size_t val);
	int setDYX_START_ADDR(size_t val);
	int setYC_START_ADDR(size_t val);
	int setSYK_START_ADDR(size_t val);
	int setDYK_START_ADDR(size_t val);
	int setYM_START_ADDR(size_t val);
	int setHIS_START_ADDR(size_t val);

	int setFrameLenLength(unsigned short val);
	int setIGramCounterLength(unsigned short val);
	int setFrameTypeLength(unsigned short val);
	int setInfoNumLength(unsigned short val);
	int setTransReasonLength(unsigned short val);
	int setAsduAddrLength(unsigned short val);
	int setInfoAddrLength(unsigned short val);

	//timer
	int setTimeOutHeartFrame(unsigned short val);
	int setTimeOutAnyFrameRecv(unsigned short val);
	int setTimeOutIGramFrameRecv(unsigned short val);
	int setTimeOutYkSel(unsigned short val);
	int setTimeOutYkExe(unsigned short val);
	int setTimeOutYkCancel(unsigned short val);
	int setTimeOutCallPara(unsigned short val);
	int setTimeOutSetPara(unsigned short val);
	int setTimeOutYcSend(unsigned short val);

	void handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerAnyFrameRecv(const boost::system::error_code& error);
	void handle_timerIGramFrameRecv(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerCallPara(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerSetPara(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerYkSelToExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerYcSend(const boost::system::error_code& error,share_commpoint_ptr point,int startIndex);

	void ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYkExe(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerCallPara(share_commpoint_ptr point,bool bContinue = false,unsigned short val = 0);
	void ResetTimerSetPara(share_commpoint_ptr point,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYkSelToExe(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYcSend(share_commpoint_ptr point,int YcSum,int startIndex,bool bContinue = false,unsigned short val = 0);

	//send frame assemble
	int AssembleSGram(size_t bufIndex, unsigned char * buf);
	int AssembleStartDTAct(size_t bufIndex, unsigned char * buf);
	int AssembleStartDTCon(size_t bufIndex, unsigned char * buf);
	int AssembleStopDTAct(size_t bufIndex, unsigned char * buf);
	int AssembleStopDTCon(size_t bufIndex, unsigned char * buf);
	int AssembleTestFRAct(size_t bufIndex, unsigned char * buf);
	int AssembleTestFRCon(size_t bufIndex, unsigned char * buf);
	int AssembleDoubleYKSelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason);
	int AssembleDoubleYKExeCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason);
	int AssembleDoubleYKCancelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code,unsigned char trans_reason);
	int AssembleDoubleYKOver(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code);
	int AssembleSingleYKSelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason);
	int AssembleSingleYKExeCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no, unsigned char yk_code,unsigned char trans_reason);
	int AssembleSingleYKCancelCon(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code,unsigned char trans_reason);
	int AssembleSingleYKOver(size_t bufIndex, unsigned char * buf, share_pristation_ptr pristationPtr,unsigned short yk_no,unsigned char yk_code);
	int AssembleSynTimeCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,boost::posix_time::ptime time);
	int AssembleCallDataCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	int AssembleCallDataOver(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	int AssembleCallYMCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	int AssembleCallYMOver(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	int AssembleAllSingleYX(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num);
	int AssembleAllDoubleYX(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num);
	int AssembleAllYCWithVaild(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num);
	int AssembleAllYM(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num);
	int AssembleSingleCOS(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CCosPoint * cosBuf,size_t info_num);
	int AssembleDoubleCOS(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CCosPoint * cosBuf,size_t info_num);
	int AssembleSingleSOE(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CSoePoint * soeBuf,size_t info_num);
	int AssembleDoubleSOE(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CSoePoint * soeBuf,size_t info_num);
	int AssembleYcVar(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CYcVarPoint * ycvarBuf,size_t info_num);
	int AssembleYcVarWithVaild(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,DataBase::CYcVarPoint * ycvarBuf,size_t info_num);
	int AssembleEndInit(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	int AssembleResetCMDCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,unsigned char QRP);
	int AssembleSingleSYxData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num);
	int AssembleSingleDYxData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num);
	int AssembleSingleYcData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num);
	int AssembleSingleYmData(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int startIndex, int info_num);
	int AssembleThansPubKeyCon(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	int AssembleSendYcByTimer(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,int & startIndex,size_t info_num);
	int AssembleSendYcIByTimer(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr,CCmd cmd);
		
	//recv frame parse
	int ParseSingleYKSel(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseSingleYKExe(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseSingleYKCancel(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseDoubleYkSel(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseDoubleYkExe(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseDoubleYkCancel(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseCallAllData(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseCallAllYMData(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseSynTime(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseReadCMD(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseResetCMD(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseYKKeyError(unsigned char * buf,share_pristation_ptr pristationPtr,size_t TransReason,unsigned char Data_Code,bool DoubleFlag);
	int ParseTransPubKey(unsigned char * buf,share_pristation_ptr pristationPtr,size_t exceptedBytes);
	int ParseFrame_S(unsigned char * buf);
	int ParseFrame_U(unsigned char * buf);
	
	//yk para
	int CreateYkFramePara(bool bCancel,bool bDouble,unsigned char yk_type,unsigned char return_code,unsigned char & yk_ode,unsigned char & trans_reason);

protected:
	size_t SYX_START_ADDR_;                              //单点yx起始地址
	size_t DYX_START_ADDR_;                              //双点yx起始地址
	size_t YC_START_ADDR_;                               //yc起始地址
	size_t SYK_START_ADDR_;                              //单点yk起始地址
	size_t DYK_START_ADDR_;                              //双点yk起始地址
	size_t YM_START_ADDR_;                               //ym起始地址
	size_t HIS_START_ADDR_;                              //历史数据起始地址

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

	typeInfoNumLimit INFONUM_LIMIT_ALLYXFRAME_;
	typeInfoNumLimit INFONUM_LIMIT_ALLYCFRAME_;
	typeInfoNumLimit INFONUM_LIMIT_ALLDDFRAME_;
	typeInfoNumLimit INFONUM_LIMIT_COSFRAME_;
	typeInfoNumLimit INFONUM_LIMIT_SOEFRAME_;
	typeInfoNumLimit INFONUM_LIMIT_YCVARFRAME_;

	unsigned short IFrameRecvCounter_;                  //收报文序列号计数器
	unsigned short IFrameSendCounter_;                  //发报文序列号计数器

	unsigned short timeOutSetPara_;
	timerPtr timerSetPara_;

private:
	enum s104Para
	{
		DEFAULT_SUM_K = 16,                                     //104规约中发送16帧I或者U报文未收到确认，判断为连接出错
		DEFAULT_SUM_W = 8,                                      //104规约中每收到8帧I报文，给对端确认

		DEFAULT_SYX_START_ADDR = 0x0001,                        //默认单点yx起始地址
		DEFAULT_DYX_START_ADDR = 0x0001,                        //默认双点yx起始地址
		DEFAULT_YC_START_ADDR = 0x4001,                         //默认yc起始地址
		DEFAULT_SYK_START_ADDR = 0x6001,                        //默认单点yk起始地址
		DEFAULT_DYK_START_ADDR = 0x6001,                        //默认双点yk起始地址
		DEFAULT_YM_START_ADDR = 0x6401,                         //默认ym起始地址
		DEFAULT_HIS_START_ADDR = 0x0000,                        //默认历史数据起始地址

		DEFAULT_FrameLenLength = 1,                             //默认报文长度标识的字节长度
		DEFAULT_IGramCounterLength = 2,
		DEFAULT_FrameTypeLength = 1,                            //默认报文类型标识的字节长度
		DEFAULT_InfoNumLength = 1,                              //默认信息体数目标识的字节长度
		DEFAULT_TransReasonLength = 2,                          //默认传送原因标识的字节长度
		DEFAULT_AsduAddrLength = 2,                             //默认装置地址标识的字节长度
		DEFAULT_InfoAddrLength = 3,                             //默认信息体地址标识的字节长度

		DEFAULT_INFONUM_LIMIT_ALLYXFRAME = 64,
		DEFAULT_INFONUM_LIMIT_ALLYCFRAME = 48,
		DEFAULT_INFONUM_LIMIT_ALLDDFRAME = 28,
		DEFAULT_INFONUM_LIMIT_COSFRAME = 32,
		DEFAULT_INFONUM_LIMIT_SOEFRAME = 16,
		DEFAULT_INFONUM_LIMIT_YCVARFRAME = 24,

		DEFAULT_timeOutAnyFrameRecv = 150,
		DEFAULT_timeOutIGramFrameRecv = 10,
		DEFAULT_timeOutHeartFrame = 10,
		MIN_timeOutHeartFrame = 1,
		DEFAULT_timeOutYkSel = 30,
		DEFAULT_timeOutYkExe = 30,
		DEFAULT_timeOutYkCancel = 30,
		DEFAULT_timeOutCallPara = 15,
		DEFAULT_timeOutSetPara = 25,
		DEFAULT_timeOutYkSelToExe = 60,  //60S后遥控自动撤销
		DEFAULT_timeOutYcSendTime = 2,   //定时上送遥测的时间，单位s
	};

	static size_t S104ObjectCounter_;

	unsigned char QOI_;

	unsigned short count_K_U_;                           //连续发送U格式报文计数器
	unsigned short count_K_I_;                           //连续发送I格式报文计数器
	unsigned short count_W_I_;                           //连续接收I格式报文计数器
	unsigned short S104_SUM_K_;                          //K计数器的基准值
	unsigned short S104_SUM_W_;                          //W计数器的基准值

	unsigned short timeOutAnyFrameRecv_;
	timerPtr timerAnyFrameRecv_;
	unsigned short timeOutIGramFrameRecv_;
	timerPtr timerIGramFrameRecv_;
	unsigned short timeOutHeartFrame_;
	timerPtr timerHeartFrame_;
	unsigned short timeOutYkSel_;
	timerPtr timerYkSel_;
	unsigned short timeOutYkExe_;
	timerPtr timerYkExe_;
	unsigned short timeOutYkCancel_;
	timerPtr timerYkCancel_;
	unsigned short timeOutYkSelToExe_;
	timerPtr timerYkSelToExe_;
	unsigned short timeOutCallPara_;
	timerPtr timerCallPara_;
	unsigned short timeOutYcSend_;
	timerPtr timerYcSend_;
	
	bool bCheckIFrameRecvCounter_;
	bool bCheckIFrameSendCounter_;
	bool bYcSendByTimer_;//遥测是否定时上送的标志
};

};//namespace Protocol
