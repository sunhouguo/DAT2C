#pragma once
#include <vector>
#include <boost/asio.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/function.hpp>
#include <iostream>
#include "../PublicSupport/LoopBuf.h"
#include "CmdQueue.h"

namespace FileSystem
{
	class CLog;
	class CMarkup;
	class CFileHandle;
}

namespace CommInterface
{
	class CCommInterface;
}

namespace DigitalSignature
{
	class CPublicKey;
	class CPrivateKey;
}

namespace Protocol {

typedef boost::shared_ptr<boost::asio::deadline_timer> timerPtr;
typedef boost::signals2::signal<int(unsigned char cmdType,unsigned char * buf,size_t length)> CommSignalType;
typedef CommSignalType::slot_type CommSlotType;
typedef std::vector<unsigned char> myframe;

const unsigned char WriteData = 1;
const unsigned char ReadData = 2;
const unsigned char OpenConnect = 3;
const unsigned char CloseConnect = 4;
const unsigned char ReConnect = 5;
const unsigned char BroadCast = 6;

//xml node api
#define strProtocolRoot "ProtocolROOT"

#define strInfoAddr "InfoAddr"
#define strSYxStartAddr "SYX_START_ADDR"
#define strDYxStartAddr "DYX_START_ADDR"
#define strYcStartAddr "YC_START_ADDR"
#define strSYkStartAddr "SYK_START_ADDR"
#define strDYkStartAddr "DYK_START_ADDR"
#define strYmStartAddr "YM_START_ADDR"
#define strHisStartAddr "HIS_START_ADDR"

#define strFrameElemLength "ElemLength"
#define strFrameLenLength "FrameLenLength"                               //报文长度标识的字节长度
#define strFrameTypeLength "FrameTypeLength"                             //报文类型标识的字节长度
#define strInfoNumLength "InfoNumLength"                                 //信息体数目标识的字节长度
#define strTransReasonLength "TransReasonLength"                         //传送原因标识的字节长度
#define strAsduAddrLength "AsduAddrLength"                               //装置地址标识的字节长度
#define strInfoAddrLength "InfoAddrLength"                               //信息体地址标识的字节长度
#define strFunTypeLength "FunTypeLength"                                 //功能类型标识的字节长度

#define strTimer "Timer"
#define strTimeOutQueryUnActivePoint "TimeOutQueryUnActivePoint"
#define strTimeOutRequireLink "TimeOutRequireLink"
#define strTimeOutCallAllData "TimeOutCallAllData"
#define strTimeOutCallAllDD "TimeOutCallAllDD"
#define strTimeOutSynTime "TimeOutSynTime"
#define strTimeOutHeartFrame "TimeOutHeartFrame"
#define strTimeOutYkSel "TimeOutYkSel"
#define strTimeOutYkExe "TimeOutYkExe"
#define strTimeOutYkCancel "TimeOutYkCancel"
#define strTimeOutCallPara "TimeOutCallPara"
#define strTimeOutSetPara "TimeOutSetPara"
#define strHeartTimeOutResetLink "HeartTimeOutResetLink"

class CProtocol :
	public CCmdQueue
{
public:
	CProtocol(boost::asio::io_service & io_service);
	virtual ~CProtocol(void);

public:
	//cmd api
	int AddSendCmdVal(CCmd val);
	int AddSendCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint);
	int AddSendCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val);
	int AddOnlySendCmdWithoutVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val);
	int AddOnlySendCmdByCmdType(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val);
	
	//comm points api
	int AddCommPoint(share_commpoint_ptr val);
	int AddCommPoint(weak_commpoint_ptr val);
	int DelCommPoint(int index);
	weak_commpoint_ptr getCommPoint(int index);
	int getCommPointIndexByAddrCommType(unsigned char commTypeVal,unsigned short addrVal,unsigned short addrLen = 0);
	int getCommPointSum();
	void ClearCommPoint();
	int DisableAllCommPoints();
	
	//遍历通讯结点
	share_commpoint_ptr getFirstCommPoint();
	share_commpoint_ptr getNextCommPoint(unsigned char pointType,boost::logic::tribool bCommActive,size_t curIndex);
	share_commpoint_ptr getNextCommPoint(unsigned char pointType,boost::logic::tribool bCommActive,share_commpoint_ptr point);
	share_commpoint_ptr getNextCommPointBySelfDef(unsigned char pointType,boost::function<bool(share_commpoint_ptr)> CheckTrueFunC,size_t curIndex);
	share_commpoint_ptr getNextCommPointBySelfDef(unsigned char pointType,boost::function<bool(share_commpoint_ptr)> CheckTrueFunC,share_commpoint_ptr point);

	//recv and send frame main api
	int RecvProcess(unsigned char * buf, size_t count);
	int SendProcess(unsigned char * buf, CCmd & cmd);

	//virtual init api
	virtual int InitProtocol();
	virtual void UninitProtocol();

	//virtual xml cfg api
	virtual int LoadXmlCfg(std::string filename) = 0;
	virtual void SaveXmlCfg(std::string filename) = 0;
	int LoadXmlCfg(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);

	//sig api
	int ConnectCmdRecallSig(CmdRecallSlotType slotVal);
	int DisconnectCmdRecallSig();
	int ConnectSubAliveSig();
	int DisconnectSubAliveSig();
	bool getSubAliveSigConnection();
	//int ConnectSubYkSig();
	//int DisconnectSubYkSig();
	virtual void ProcessSubAliveSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);
	//virtual void ProcessSubYkSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,size_t index);

	SigConnection ConnectCommSingal(CommSlotType slotVal);

	//CFileHnadle  api
protected:
	unsigned long FileHandleGetRemainLength(void);
	unsigned long FileHandleGetTotalLength(void);
	std::string FileHandleGetFileName(void);
	unsigned int FileHandleSetTotalLength(unsigned long Totallen);
	int FileHandleWrite(void);
	int FileHandleWriteByByte(void);
	int FileHandleRead(void);
	int FileHandleReadFor533Pro(void);
	int FileHandleOutFile(unsigned char * filedata,int length);
	int FileHandleGetFile(unsigned char * filedata,int length);
	int FileHandleBegain(std::string name);
	int FileHandleFinish(void);

	//frame api
	virtual int MatchFrameHead(size_t & exceptedBytes);
	virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes) = 0;
	virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes) = 0;
	virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes) = 0;                  //返回通讯节点在容器中的序号

	virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd) = 0;
	virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd) = 0;
	virtual int AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd) = 0;

	bool CheckFrameRepeatOutCount(size_t & count);

	int getFrameBufLeft();

	//comm api
	int WriteDatas(unsigned char * buf,size_t length);
	int WriteBroadCast(unsigned char * buf,size_t length);
	int ReConnnectChannel();

	//protocol api
	share_commpoint_ptr getWaitingForAnswer();
	void setWaitingForAnswer(share_commpoint_ptr val);
	bool getbConsecutiveSend();
	void setbConsecutiveSend(bool val);
	virtual int QueryUnAliveCommPoint(share_commpoint_ptr point);
	int getCommPointIndexByPtr(share_commpoint_ptr point);
	int getMeanvalueOfPointsSum(unsigned short minVal,unsigned short val);

	//cmd api
	void handle_SendCmd();
	void ClearFrameRepeatFlag();
	void ClearWaitAnswerFlag();
	typeCmd getLastSendCmd();
	typeCmd getLastRecvCmd();
	int setLastSendCmd(typeCmd val);
	int setLastRecvCmd(typeCmd val);
	int getLastSendPointIndex();
	int getLastRecvPointIndex();
	int setLastSendPointIndex(int val);
	int setLastRecvPointIndex(int val);
	int getLastFrameLength();

	//commpint api
	int MathcCommPoint(weak_commpoint_ptr val);
	int InitAllCommPoints();
	int EnableCommPoint(share_commpoint_ptr point);
	//int setCommPointsDataBaseQuality(bool active);
	//bool getAllCommPointAlive();

	//log api
	int AddStatusLog(std::string strVal);
	int AddStatusLogWithSynT(std::string strVal);
	int RecordFrameData(const unsigned char * buf,size_t count,unsigned char datatype);
	int EnableStatusLog(std::string filename,std::string filetype,std::string limit = "");
	int EnableFrameLog(std::string filename,std::string filetype,std::string limit = "");
	void DisableStatusLog();
	void DisableFrameLog();

	//timer api
	void StopAllTimer();
	void AddTimer(timerPtr val);
	//void handle_timerWaitForAnswer(const boost::system::error_code& error,size_t index);
	//void ResetTimerWaitForAnswer(size_t commpointIndex,bool bContinue = true,unsigned short val = 0);
	void handle_timerWaitForAnswer(const boost::system::error_code& error,share_commpoint_ptr point);
	void ResetTimerWaitForAnswer(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
	void handle_timerConsecutiveSend(const boost::system::error_code& error);
	void ResetTimerConsecutiveSend(bool bContinue = true,unsigned short val = 0);

	bool getPrintFrameTerm();
	int setPrintFrameTerm(bool val);
	bool getPrintStatusTerm();
	int setPrintStatusTerm(bool val);
	int PrintInfoToTerm(std::string strVal);
	bool getClearQuality();
	int setClearQuality(bool val);
	bool getClearEvent();
	int setClearEvent(bool val);

	//boradcast api
	bool getBroadCastSend();
	int setBroadCastSend(bool val);

	//key api
	int getSignStartIndex();

	int CalcSecretDataLength(unsigned char * buf,size_t keyIndex,bool bCalcByFrame = true);

	int EnablePubKey();
	int EnablePriKey();
	bool CheckPubKey();
	bool CheckPriKey();

	int encrypt(size_t bufBegin,size_t bufCount,unsigned char * buf);
	int decrypt(size_t bufBegin,size_t bufCount,size_t dsStartIndex,unsigned char * buf);

	int CheckPlusHead(unsigned char * buf,size_t & exceptedBytes);
	int CheckPlusTail(unsigned char * buf,size_t exceptedBytes);
	int ParseFrame_Plus(unsigned char * buf,size_t exceptedBytes);

	int AssembleCheckPubKeyCon(size_t bufIndex,unsigned char * buf,share_commpoint_ptr pristationPtr,bool bConAct);
	int AssembleUpdatePubKeyCon(size_t bufIndex,unsigned char * buf,share_commpoint_ptr pristationPtr,bool bConAct);

private:
	int AddFrameLog(std::string strVal);
	int AddFrameLogWithSynT(std::string strVal);
		
	//key api
	std::string getSecretKeyPath();
	std::string getSecretKeyType();
	int setSecretKeyType(std::string val);
	int setSecretKeyPath(std::string val);

	int setSignStart(int SignStart);

	//std::string TransSecretKeyTypeToString(unsigned char val);
	//unsigned char TransSecretKeyTypeFromString(std::string val);

	int NotifySendCmd();
	
protected:
	enum errorType
	{
		NO_SYN_HEAD = 1,
		LESS_RECV_BYTE = 2,

		max_send_length = 1024,       //发送缓冲区最大长度
		max_recv_length = 4096,       //接收缓冲区最大长度

		WaitforanswerTimeOut = 3,     //等到应答报文超时的时间的默认值 单位：秒
		frameRepeatSum = 2,           //报文需要重发的总次数的默认值
		LostAnswerTimesOut = 30,      //多少次无应答报文以后视为通讯中断的次数默认值
		LostRecvTimeOut = 180,        //多长时间未收到报文以后视为通讯中断的时长默认值，单位：秒
		ConsecutiveSendTimeOut = 200  //连续发送两帧报文之间需要的时间间隔的默认值 单位：毫秒
	};
	
	size_t ProtocolObjectIndex_;

	//std::vector<weak_pristation_ptr> pristations_; //属于本通道的主站指针集合
	//std::vector<weak_terminal_ptr> terminals_;     //属于本通道的终端指针集合
	std::vector<weak_commpoint_ptr> commPoints_;     //属于本通道的通讯结点指针集合
	//int iCurCommpointIndex_;                      //当前处理的通讯结点序号
	share_commpoint_ptr CurWaitCommpointPtr_;        //当前需要等待应答的通讯结点指针，为空表示没用需要应答的结点。

	bool bActiveDataUp_;                             //该规约是否是主动上送数据的

	bool bActiveRepeatFrame_;                        //该规约是否需要重发请求报文
	size_t iFrameRepeatSum_;                         //报文需要重发的总次数
	size_t iLostAnswerTimesOut_;                     //多少次无应答报文以后视为通讯中断

	//bool bWaitingForAnswer_;                       //需要等待报文回应
	unsigned short timeOutWaitforAnswer_;            //等到应答报文超时的时间 单位：秒

	bool bConsecutiveSend_;                          //需要等待休息时间才发下一帧报文
	unsigned short timeOutConsecutiveSend_;          //连续发送两帧报文之间需要的时间间隔 单位：毫秒

	bool bActiveDataRecv_;                           //该规约是否需要靠接收到的数据来判定结点的通讯状况
	unsigned short timeOutLostRecv_;                 //多长时间未收到报文以后视为通讯中断，单位：秒

	bool bRepeatLastFrame_;                          //重发上一帧报文
	size_t iFrameRepeatCount_;                       //当前报文已经重发的次数
	size_t SynCharNum_;                              //报文同步头的长度
	bool bBroadCastSend_;                            //是否发送广播报文

	bool bClearDataBaseQuality_;                     //是否在规约初始化阶段将各个数据库的品质描述值置为失效，这样收到数据报文后需要在具体规约中手动将品质描述置为有效。
	bool bClearEventLoadPtr_;                        //是否在规约初始化阶段重置CPristation的告警事件的Load指针，默认false

	bool bInitQuality_;                              //使规约下的终端通讯点的数据品质描述受初始化标志:CCommPoint::bInitCommPoint_影响
	
	std::string SecretKeyType_;                      //报文使用密钥加密解密
	std::string KeyPath_;                            //密钥文件路径名称
	int SignStart_;                                  //签名报文开始位置
	
	CommSignalType CommSig_;
	CmdRecallSignalType CmdConSig_;
	SigConnection CommSigConnection_;
	SigConnection CmdConSigConnection_;
	SigConnection SubAliveSigConnection_;
	//boost::signals::connection SubYkSigConnection_;

	//bool bFirstCallAllData_;                         //规约是否第一次召唤数据

	int max_frame_length_;

	//PublicSupport::CLoopBuf  loopbuf_;                   //接收报文的循环缓冲区
	boost::scoped_ptr<PublicSupport::CLoopBuf> recv_buf_;   //接收报文的循环缓冲区

	//unsigned char send_data_[max_send_length];           //发送报文缓冲区
	boost::scoped_array<unsigned char> send_buf_;         //发送报文缓冲区
	int iLastFrameLength_;                              //上一帧发送报文的长度

private:
	//定时器
	boost::asio::deadline_timer timerWaitForAnswer_;   //等待应答报文的定时器 时长单位：秒
	boost::asio::deadline_timer timerConsecutiveSend_; //连续发送两帧报文之间需要的时间间隔的定时器 时长单位：毫秒
	std::vector<timerPtr> timers_;                     //具体规约使用的定时器集合 时长单位：秒

	//IO服务
	boost::asio::io_service & io_service_;

	typeCmd LastSendCmd_;
	int LastSendPointIndex_;
	typeCmd LastRecvCmd_;
	int LastRecvPointIndex_;

	//指向日志文件的指针
	boost::scoped_ptr<FileSystem::CLog> statusLog_;
	boost::scoped_ptr<FileSystem::CLog> frameLog_;
	boost::scoped_ptr<FileSystem::CFileHandle> FileHandle_;
	//std::string statusLogName_;
	//std::string frameLogName_;
	bool bPrintFrame_;
	bool bPrintStatus_;

	//key
	boost::shared_ptr<DigitalSignature::CPublicKey> pubKey_;
	boost::shared_ptr<DigitalSignature::CPrivateKey> priKey_;
};

} //Protocol
