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

namespace Protocol{

class CS101:
	public CProtocol
{
public:
	CS101(boost::asio::io_service & io_service);
	~CS101(void);

	virtual int LoadXmlCfg(std::string filename);
	virtual void SaveXmlCfg(std::string filename);

protected:
	virtual int MatchFrameHead( size_t & exceptedBytes );
	virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes);
	virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes);
	virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes);

	virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd);
	virtual int AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd);

	virtual int InitProtocol();
	virtual void UninitProtocol();

	int ConnectSubYkSig(share_commpoint_ptr point);
	int DisconnectSubYkSig(share_commpoint_ptr point,bool bForceClose);
	virtual void ProcessSubYkSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);

	void InitFrameLocation(size_t FrameHead);

	//protocol func api
	int getAddrByRecvFrame(unsigned char * buf);
	unsigned char getACD(share_pristation_ptr pristationPtr);
	int CheckACD(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	unsigned char getFCB(share_pristation_ptr pristationPtr);
	int setFCB(share_pristation_ptr pristationPtr,unsigned char val);
	int CheckFCB(unsigned char FCV,unsigned char FCB,typeCmd CurCmd,share_pristation_ptr pristationPtr);

	int ParseShortFrame(unsigned char * buf, share_pristation_ptr pristationPtr);
	int ParseLongFrame(unsigned char * buf, share_pristation_ptr pristationPtr,size_t exceptedBytes);

private:
	void InitObjectIndex();

	void InitDefaultStartAddr();
	void InitDefaultFrameElem();
	void InitDefaultTimeOut();
	void InitDefaultTimer(boost::asio::io_service & io_service);

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

	//timer
	int setTimeOutYkSel(unsigned short val);
	int setTimeOutYkExe(unsigned short val);
	int setTimeOutYkCancel(unsigned short val);
	int setTimeOutCallPara(unsigned short val);
	int setTimeOutSetPara(unsigned short val);

	void handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerYkExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerYkCancel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);
	void handle_timerCallPara(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerSetPara(const boost::system::error_code& error,share_commpoint_ptr point);
	void handle_timerYkSelToExe(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);

	void ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYkExe(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYkCancel(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);
	void ResetTimerCallPara(share_commpoint_ptr point,bool bContinue = false,unsigned short val = 0);
	void ResetTimerSetPara(share_commpoint_ptr point,bool bContinue = false,unsigned short val = 0);
	void ResetTimerYkSelToExe(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);

	//send frame assemble
	int AssembleLinkStatus(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	int AssembleConfirmNack(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
	int AssembleConfirmAck(size_t bufIndex,unsigned char * buf,share_pristation_ptr pristationPtr);
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

	//recv frame parse
	int ParseResetLink(unsigned char * buf, share_pristation_ptr pristationPtr);
	int ParseRequireLink(unsigned char * buf, share_pristation_ptr pristationPtr);
	int ParseCallPrimaryDatas(unsigned char * buf, share_pristation_ptr pristationPtr);
	int ParseCallSecondaryDatas(unsigned char * buf, share_pristation_ptr pristationPtr);
	int ParseSingleYKSel(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseSingleYKExe(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseSingleYKCancel(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseDoubleYkSel(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseDoubleYkExe(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseDoubleYkCancel(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseCallAllData(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseCallAllYMData(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseSynTime(unsigned char * buf,share_pristation_ptr pristationPtr);
	int ParseYKKeyError(unsigned char * buf,share_pristation_ptr pristationPtr,size_t TransReason,unsigned char Data_Code,bool DoubleFlag);

	//yk para
	int CreateYkFramePara(bool bCancel,bool bDouble,unsigned char yk_type,unsigned char return_code,unsigned char & yk_ode,unsigned char & trans_reason);
	
protected:
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

private:
	enum s101Para
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

		DEFAULT_timeOutYkSel = 30,
		DEFAULT_timeOutYkExe = 30,
		DEFAULT_timeOutYkCancel = 30,
		DEFAULT_timeOutCallPara = 15,
		DEFAULT_timeOutSetPara = 25,
		DEFAULT_timeOutYkSelToExe = 60,  //60S后遥控自动撤销
	};

	static size_t S101ObjectCounter_;

	unsigned char QOI_;

	size_t SYX_START_ADDR_;                              //单点yx起始地址
	size_t DYX_START_ADDR_;                              //双点yx起始地址
	size_t YC_START_ADDR_;                               //yc起始地址
	size_t SYK_START_ADDR_;                              //单点yk起始地址
	size_t DYK_START_ADDR_;                              //双点yk起始地址
	size_t YM_START_ADDR_;                               //ym起始地址
	size_t HIS_START_ADDR_;                              //历史数据起始地址

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
	unsigned short timeOutSetPara_;
	timerPtr timerSetPara_;
};

};//namespace Protocol
