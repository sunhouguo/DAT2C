#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <boost/date_time/time_duration.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"
#include "../CommInterface/CommPara.h"

//自定义通讯结点类型
const unsigned char PRISTATION_NODE = 1;
const unsigned char TERMINAL_NODE = 2;

//通知事件类型
const unsigned char COMM_EVENT = 1;
const unsigned char YX_EVENT = 2;
const unsigned char YK_EVENT = 3;
const unsigned char SELF_EVENT = 4;

namespace FileSystem
{
	class CMarkup;
}

namespace Protocol
{
	class CCmd;
}

namespace CommInterface 
{
	class CCommInterface;
	class CServerInterface;
}

namespace DataBase {

typedef boost::shared_ptr<CommInterface::CCommInterface> share_comm_ptr;
typedef boost::shared_ptr<CommInterface::CServerInterface> share_server_ptr;
typedef boost::variant<share_comm_ptr,share_server_ptr> share_pointval_ptr;

class CCommPoint
{
public:
	//CCommPoint();
	CCommPoint(boost::asio::io_service & io_service);
	virtual ~CCommPoint(void);

	int EnableCommunication(CmdRecallSlotType slotVal,share_pointval_ptr ptr = share_pointval_ptr(),bool bNewPtr = true);
	void DisableCommunication();

	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml);

	//Addr api
	typeAddr getAddr();
	int setAddr(typeAddr val);
	typeAddr getBroadCastAddr();
	int setBroadCastAddr(typeAddr val);
	bool getbAcceptBroadCast();
	int setbAcceptBroadCast(bool val);
	
	void ClearServerPtr();
	void SetServerPtr(share_server_ptr valPtr);
	share_server_ptr getServerPtr();

	void ClearCommPtr();
	void ResetCommPtr(CommInterface::CCommInterface * valPtr);
	void ResetCommPtr(share_comm_ptr valPtr);
	share_comm_ptr getCommPtr();

	int AddCmdVal(Protocol::CCmd val);
	int AddCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val);
	int AddCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint);
	void clearCmdQueue();
	int getCmdQueueSum();
	int getMaxPriopriyCmd(Protocol::CCmd & dstVal);

	//虚函数接口
	virtual share_commpoint_ptr getSelfPtr() = 0;
	virtual SigConnection ConnectSubAliveSig(CmdRecallSlotType slotVal) = 0;
	virtual SigConnection ConnectSubTempSig(CmdRecallSlotType slotVal) = 0;
	//virtual void ClearDataBaseQuality(bool active);

	//穿越API
	void AddSelfPointToProtocol();
	//void AddSelfPointToServerPtr();
	int ResetTimerRecv(size_t LostRecvTimeOut);
	void handle_timerRecv(const boost::system::error_code& error);

	//xml cfg api
	//std::string TransCommChannelTypeToString(unsigned char val);
	//char TransCommChannelTypeFromString(std::string val);
	//std::string TransProtocolTypeToString(unsigned short val);
	//short TransProtocolTyeFromString(std::string val);
	
	//comm api
	virtual bool getCommActive();
	virtual void setCommActive(bool val);
	bool getInitCommPointFlag();
	void setInitCommPointFlag(bool val);
	bool getSynTCommPointFlag();
	void setSynTCommPointFlag(bool val);
	bool getDelayCommPointFlag();
	void setDelayCommPointFlag(bool val);
	bool CheckLostAnserTimesPlus(size_t LostAnswerTimesOut);

	unsigned char getCommPointType();

	//cmd recall api
	int ConnectCmdRecallSig(CmdRecallSlotType slotVal);
	int DisconnectCmdRecallSig();

	int ConnectCmdRelaySig(CmdRecallSlotType slotVal);
	int DisconnectCmdRelaySig(bool bForceClose);
	//bool getSubTempSigConnection();
	void ProcessRelayCmd(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);

	std::string getLocalPort();
	std::string getRemoteID();
	bool MatchRemoteIP(std::string val);
	CommInterface::CCommPara & getCommPara();

	virtual int InitLocalServices(CmdRecallSlotType slotVal,bool SlotReady);
	virtual void UnInitLocalServices();

	//表示通讯点当前的通讯状态
	int NotifyEventStatus(typeCmd EventType,unsigned char ReturnCode);

	boost::posix_time::time_duration getTdDiff();
	int setTdDiff(boost::posix_time::time_duration val);

	//encrypt api
	bool getEncryptOutside();
	int setEncryptOutside(bool val);

private:
	bool MatchRemoteID(std::string val);
	int setRemoteID(std::string val);

	void MulCmdRelaySigCounterPlus();
	bool MulCmdRelaySigCounterCut();
	void ClearMulCmdRelaySigCounter();

	int ConnectEventStatusSig(CmdRecallSlotType slotVal);
	void DisconnectEventStatusSig();

protected:
	bool bCommActive_;                          //该通讯结点是否通讯正常
	size_t iLostAnswerTimes_;                   //该结点未响应请求的次数
	boost::asio::deadline_timer timerRecv_;     //接收报文超时定时器
	
	unsigned char pointType_;                   //通讯结点类型
	
	share_comm_ptr commPtr_;                    //指向通讯通道的指针
	share_server_ptr serverPtr_;                //指向监听通道的指针

	boost::asio::io_service & io_service_;      //IO服务的引用

	typeAddr addr_;                             //通讯地址标识
	bool bAceeptBroadcast_;                     //该站点是否接收广播或者组播
	typeAddr BroadCastAddr_;                    //接收广播或者组播的地址
	bool bFcbFlag_;

	bool bDelayDownload_;                       //同步通讯结点延时标记
	bool bSynTCommPoint_;                       //同步通讯结点时间标记 
	bool bInitCommPoint_;                       //初始化通讯结点数据标记

	CmdRecallSignalType RelaySig_;              //信号中继转发
	int MulCmdRelaySigCounter_;                 //多个信号连接的计数器
	SigConnection RelaySigConnection_;
	SigConnection SubTempSigConnection_;

	CmdRecallSignalType EventStatusSig_;         //事件状态输出信号
	SigConnection EventStatusConnection_;        //事件状态输出连接 
	
	//事件状态注册的标识点号
	int CommRegisterNO_;
	int YxReisterNO_;
	int YkRegisterNO_;
	int SelfRegisterNO_;

	CommInterface::CCommPara commPara_; //通讯参数

	boost::posix_time::time_duration td_diff_;

	bool bEncryptOutside_;
};

};  //namespace DataBase
