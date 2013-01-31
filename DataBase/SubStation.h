#pragma once
#include <vector>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include "CosPoint.h"
#include "SoePoint.h"
#include "YcVarPoint.h"
#include "../PublicSupport/CircBufDataBase.h"
#include "YxDB.h"
#include "YcDB.h"
#include "YmDB.h"
#include "YkDB.h"
#include "CalcYxDB.h"

const std::string MainCfgFile = "sub.xml";
const std::string ver = "v1.33-2012.01.07";

namespace CentralizedFA
{
	class CCentralizedDA;
}

namespace FileSystem
{
	class CLog;
}

namespace LocalDrive
{
	class CWatchDog;
	class CDIO;
	class CBluetooth;
	class CBatteryActive;
	class CLightDriver_TPE3000;
	class CIEEE1588_TPE3000;
}

namespace PublicSupport
{
	class CEfficientRouteTab;
	class CSpaceRouteTab;
}

namespace Protocol
{
	class CCmd;
}

namespace DataBase {

const std::string strSynImm = "SynImm";
const std::string strCallImm = "CallImm";
const std::string strGeneralCmd = "GeneralCmd";
const std::string strSpeCmd = "SpeCmd";

class CYxPoint;
class CCalcYxPoint;
class CYkPoint;
class CYmPoint;
class CYcPoint;

class CBF533Terminal;

class CDAOperateSub;

class CYkRecord;
class CCosRecord;
class CSoeRecord;

class CLine;

typedef boost::shared_ptr<CBF533Terminal> share_bf533_ptr;

class CSubStation
{
public:
	CSubStation(void);
	~CSubStation(void);

	int InitSubStation();
	void UninitSubStation();
	typeAddr getAddr();
	int setAddr(typeAddr val);

	//cos api
	void InitCosLoadPtr(size_t & loadPtr);
	int getCosPointNum(size_t loadPtr);
	int putCosPoint(CCosPoint val);
	int putCosPoint(size_t yxIndex,typeYxval yxVal,typeYxtype yxType,typeYxQuality yxQuality = 0);
	int putCosPoint(size_t terminalIndex,size_t terminalyxIndex,typeYxval yxVal,typeYxtype yxType,typeYxQuality yxQuality = 0);
	CCosPoint getCosPoint(size_t loadPtr);                    //取指定点号的Point值，不移动loadPtr指针,需要自行维护loadPtr的合法性
	int saveCosPoints(const CCosPoint * cosBuf,size_t count); //这个cos接口侧重存入的效率，所以是不校验遥信点号的合法性，以及自动按cos的yx值去设置yx库的相应yx值的,这些工作需要调用程序自行完成。
	int loadCosPoints(size_t & loadPtr,CCosPoint * cosBuf,size_t count);
	int copyCosPoints(size_t loadPtr,CCosPoint * cosBuf,size_t count);
	int moveCosLoadPtr(size_t & loadPtr,size_t count);
	int CheckEffectCalcCos(size_t index);

	//soe api
	void InitSoeLoadPtr(size_t & loadPtr);
	int getSoePointNum(size_t loadPtr);
	int putSoePoint(CSoePoint val);
	int putSoePoint(size_t yxIndex,typeYxval yxVal,typeYxtype yxType,ptime time,typeYxQuality yxQuality = 0);
	int putSoePoint(size_t terminalIndex,size_t terminalyxIndex,typeYxval yxVal,typeYxtype yxType,ptime time,typeYxQuality yxQuality = 0);
	CSoePoint getSoePoint(size_t loadPtr);                    //取指定点号的Point值，不移动loadPtr指针,需要自行维护loadPtr的合法性
	int saveSoePoints(const CSoePoint * soeBuf,size_t count); //这个soe接口侧重存入的效率，所以是不校验遥信点号的合法性，以及自动按soe的yx值去设置yx库的相应yx值的,这些工作需要调用程序自行完成。
	int loadSoePoints(size_t & loadPtr,CSoePoint * soeBuf,size_t count);
	int copySoePoints(size_t loadPtr,CSoePoint * soeBuf,size_t count);
	int moveSoeLoadPtr(size_t & loadPtr, size_t count);
	int CheckEffectCalcSoe(size_t index,ptime time);
	
	//ycvar api
	void InitYcvarLoadPtr(size_t & loadPtr);
	int getYcvarPointNum(size_t loadPtr);
	int putYcvarPoint(CYcVarPoint val);
	int putYcvarPoint(size_t ycIndex,typeYcval ycVal,typeYxQuality yxQuality = 0,ptime time = ptime());
	int putYcvarPoint(size_t terminalIndex,size_t terminalycIndex,typeYcval ycVal,typeYxQuality yxQuality = 0,ptime time = ptime());
	CYcVarPoint getYcvarPoint(size_t loadPtr);                //取指定点号的Point值，不移动loadPtr指针,需要自行维护loadPtr的合法性
	int saveYcvarPoints(const CYcVarPoint * ycvarBuf,size_t count);
	int loadYcvarPoints(size_t & loadPtr,CYcVarPoint * ycvarBuf,size_t count);
	
	//yx api
	size_t getYxSum();
	size_t getCalcYxSum();
	size_t getSubYxSum();
	size_t getTerminalYxSum();
	int setCalcYxSum(size_t val);
	int setSubYxSum(size_t val);
	typeYxval getOriYxVal(size_t index);
	int setOriYxVal(size_t index, typeYxval val);
	//int setOriYxValDateUp(size_t index, typeYxval val);
	typeYxtype getYxType(size_t index);
	int setYxType(size_t index,typeYxtype val);
	bool getYxPolar(size_t index);
	int setYxPolar(size_t index,bool val);
	typeYxQuality getYxQuality(size_t index);
	int setYxQuality(size_t index,typeYxQuality val);
	typeYxval getFinalYxVal(size_t index);
	int AddEffectCalcYxIndex(size_t index,size_t val);
	int getEffectCalcYxSum(size_t index);
	int getEffectCalcYxIndex(size_t index,int val);

	//yc api
	size_t getYcSum();
	size_t getSubYcSum();
	size_t getTerminalYcSum();
	int setSubYcSum(size_t val);
	typeYcval getOriYcVal(size_t index);
	int setOriYcVal(size_t index,typeYcval val);
	typeYcplus getYcPlus(size_t index);
	int setYcPlus(size_t index,typeYcplus val);
	typeYcmul getYcMul(size_t index);
	int setYcMul(size_t index,typeYcmul val);
	typeYcquality getYcQuality(size_t index);
	int setYcQuality(size_t index,typeYcquality val);
	typeYcdead getYcDeadLimit(size_t index);
	int setYcDeadLimit(size_t index,typeYcdead val);
	typeFinalYcval getFinalYcVal(size_t index,bool bUseMul = false);
	CYcPoint * getYcPointPtr(int index);

	//yk api
	size_t getYkSum();
	size_t getSubYkSum();
	size_t getTerminalYkSum();
	int setSubYkSum(size_t val);
	//typeYkstatus getYkStatus(size_t index);
	//int setYkStatus(size_t index,typeYkstatus val);
	typeYktype getYkType(size_t index);
	int setYkType(size_t index,typeYkstatus val);
	bool getbHYkDouble(size_t index);
	int setbHYkDouble(size_t index,bool val);
	bool getbSYkDouble(size_t index);
	int setbSYkDouble(size_t index,bool val);
	int AddYkSelCmd(size_t index,bool bCloseOrOpen);
	int AddYkExeCmd(size_t index,bool bCloseOrOpen);
	int AddYkCancelCmd(size_t index,bool bCloseOrOpen = true);
	int AddBF533Cmd(int tIndex,Protocol::CCmd cmdVal);
	int getTerminalYcINum(int tIndex);
	CYkPoint * getYkPointPtr(int index);

	//ym api
	size_t getYmSum();
	size_t getSubYmSum();
	size_t getTerminalYmSum();
	int setSubYmSum(size_t val);
	typeYmval getOriYmVal(size_t index);
	int setOriYmVal(size_t index,typeYmval val);
	typeYmquality getYmQuality(size_t index);
	int setYmQuality(size_t index,typeYmquality val);

	//line api
	size_t getLineSum();
	CLine * getLinePtr(int index);

	//index transform
	int TerminalyxToSubyx(size_t terminalIndex,size_t yxIndex);
	int SubyxToTerminalyx(size_t yxIndex,size_t & terminalIndex,size_t & terminalyxIndex);
	int TerminalycToSubyc(size_t terminalIndex,size_t ycIndex);
	int SubycToTerminalyc(size_t ycIndex,size_t & terminalIndex,size_t & terminalycIndex);
	int TerminalykToSubyk(size_t terminalIndex,size_t ykIndex);
	int SubykToTerminalyk(size_t ykIndex,size_t & terminalIndex,size_t & terminalykIndex);
	int TerminalymToSubym(size_t terminalIndex,size_t ymIndex);
	int SubymToTerminalym(size_t ymIndex,size_t & terminalIndex,size_t & terminalymIndex);
	int TerminallineToSubline(size_t terminalIndex,size_t lineIndex);
	int SublineToTerminalline(size_t lineIndex,size_t & terminalIndex,size_t & terminallineIndex);

	//terminal api
	size_t getTerminalNum();
	int getTerminalIndexByPtr(share_terminal_ptr val);
	int setBf533Terminal(share_bf533_ptr val);
	share_bf533_ptr getBf533Terminal();

	//pristation api
	size_t getPristationNum();
	int getPristationIndexByPtr(share_pristation_ptr val);

	//comm api
	int InitCommPoints();
	void UninitCommPoints();
	size_t getCommPointNum();

    //Local api
    int InitLocalServices();
    void UninitLocalServices();
	int WriteSysTime(unsigned short year,unsigned char month,unsigned char day,unsigned char hour,unsigned char minnutes,unsigned char seconds,unsigned short milliseconds,bool bSysTerminal);

	//xml cfg api
	void SaveXmlCfg(std::string filename);
	int LoadXmlCfg(std::string filename);

	//sig api
	SigConnection ConnectSubAliveSig(CmdRecallSlotType slotVal);
	SigConnection ConnectSubTempSig(CmdRecallSlotType slotVal);

	//tab api
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> getTerminalYxTabPtr(size_t index);
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> getTerminalYcTabPtr(size_t index);
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> getTerminalYkTabPtr(size_t index);
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> getTerminalYmTabPtr(size_t index);

	boost::shared_ptr<PublicSupport::CEfficientRouteTab> getPriStationYxTabPtr(size_t index);
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> getPriStationYcTabPtr(size_t index);
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> getPriStationYkTabPtr(size_t index);
	boost::shared_ptr<PublicSupport::CEfficientRouteTab> getPriStationYmTabPtr(size_t index);

	//reboot
	void ResetDev(bool val);

	//event api
	void TrigCosEvent(size_t index,typeYxval val,bool bSingleType = true);
	void TrigSoeEvent(size_t index,typeYxval val,boost::posix_time::ptime time,bool bSingleType = true);
	void TrigCosEvent(int num);
	void TrigSoeEvent(int num);
	void TrigYcSendByTimeEvent(int num);

	//frame api
	//frame api
	int CallAllData();
	int SynAllTime();
	int CallData(int terminalIndex);
	int SynTime(int terminalIndex);

	int AddGeneralCmd(int tIndex,Protocol::CCmd cmdVal);
	int AddSpeCmd(int tIndex,Protocol::CCmd cmdVal);

private:
	//system time api
	static int WriteSystemTime(unsigned short year,unsigned char month,unsigned char day,unsigned char hour,unsigned char minnutes,unsigned char seconds,unsigned short milliseconds);

	//slot
	void ProcessRecallCmd(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);
	void ProcessEventCmd(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);
	bool CheckEventProcessReady();

	//commpoint api
	share_commpoint_ptr getCommPoint(size_t index);

	//log api
	int EnableLoadSubCfgLog(std::string fileName,std::string filetype,std::string limit);
	int EnableInitCommLog(std::string fileName,std::string filetype,std::string limit );
	int AddLoadSubCfgLog(std::string strVal);
	int AddLoadSubCfgWithSynT(std::string strVal);
	int AddInitCommLog(std::string strVal);
	int AddInitCommLogWithSynT(std::string strVal);

	//local timer api
	void handle_timerDelayLocalYkCon(const boost::system::error_code& error,typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);
	void DelayLocalYkCon(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);
	void handle_timerResetDev(const boost::system::error_code& error,bool val);
	void DelayResetDev(bool val);
	void handle_timerWriteTime2Dev(const boost::system::error_code& error,bool bContinue);
	void ResetTimerWriteTime2Dev(bool bContinue);

	//local driver api
	std::string getWatchdogStr();
	int setWatchdogStr(std::string val);
	bool getbWatchdogLog();
	int setbWatchdogLog(bool val);
	void ReFeedWatchDog();
	std::string getDioStr();
	int setDioStr(std::string val);
	std::string getDaStr();
	int setDaStr(std::string val);
	std::string getBaStr();
	int setBaStr(std::string val);
	std::string getLightStr();
	int setLightStr(std::string val);

	int EnableWatchDog();
	int EnableBlueTooth();
	int EnableDIO();
	int EnableDA();
	int EnableBA();
	int EnableLight();
	int EnableIEEE1588();

	void DisableWatchDog();
	void DisableBluetooth();
	void DisableDIO();
	void DisableDA();
	void DisableBA();
	void DisableLight();
	void DisableIEEE1588();

	//record data api
	int AddYkRecord(unsigned char recordType,int yk_no,bool bCloseOrOpen);
	int AddCosRecord(const CCosPoint & cos);
	int AddSoeRecord(const CSoePoint & soe);
	void InitDefaultRecord();

	int ConnectEventStatusSig(CmdRecallSlotType slotVal);
	void DisconnectEventStatusSig();
	int NotifyEventStatus(typeCmd EventType,unsigned char ReturnCode);
		
private:
	enum
	{
		DefaulAddr = 1,

		DefaultYkConDelay = 1,            //子站层遥控操作回应等待时间，单位：秒
		DefaultResetDevDelay = 3,         //装置重启等待时间，单位：秒
		DefaultWriteTime2DevTime = 60,    //将系统时间写入硬件的时间间隔，单位：分钟

		DefaultSoeRecordLimit = 1024, 
		DefaultCosRecordLimit = 1024,
		DefaultYkRecordLimit = 1024,
	};

	//Local services
	bool bWatchdogLog_;
	std::string strWatchdog_;
	boost::scoped_ptr<LocalDrive::CWatchDog> watchdog_;

	std::string strDIO_;
	boost::scoped_ptr<LocalDrive::CDIO> dio_;
	
	boost::scoped_ptr<LocalDrive::CBluetooth> bluetooth_;

	std::string strBA_;
	boost::scoped_ptr<LocalDrive::CBatteryActive> ba_;

	std::string strLight_;
	boost::scoped_ptr<LocalDrive::CLightDriver_TPE3000> light_;

	unsigned short timeOutWriteCmosClock_;
	boost::scoped_ptr<LocalDrive::CIEEE1588_TPE3000> ieee1588_;

	//algorithm
	std::string strda_;
	boost::scoped_ptr<CentralizedFA::CCentralizedDA> da_;
	boost::scoped_ptr<DataBase::CDAOperateSub> da_op_;
	
	//io
	boost::asio::io_service io_service_;
	boost::scoped_ptr<boost::asio::io_service::work> work_;
	
	//timer
	boost::asio::deadline_timer LocalYkConTimer_;
	boost::asio::deadline_timer ResetDevTimer_;
	boost::asio::deadline_timer WriteTime2Dev_;
	
	//attribute
	typeAddr addr_;
	boost::scoped_ptr<FileSystem::CLog> LoadSubCfgLog_;
	boost::scoped_ptr<FileSystem::CLog> InitCommLog_;
	
	//database
	//PublicSupport::CDynamicArrayDataBase<CYxPoint> sub_yxDB_;
	//PublicSupport::CDynamicArrayDataBase<CCalcYxPoint> calc_yxDB_;
	//PublicSupport::CDynamicArrayDataBase<CYkPoint> sub_ykDB_;
	//PublicSupport::CDynamicArrayDataBase<CYcPoint> sub_ycDB_;
	//PublicSupport::CDynamicArrayDataBase<CYmPoint> sub_ymDB_;

	CYxDB sub_yxDB_;
	CYcDB sub_ycDB_;
	CYmDB sub_ymDB_;
	CYkDB sub_ykDB_;
	CCalcYxDB calc_yxDB_;

	PublicSupport::CCircBufDataBase<CCosPoint> cosDB_;
	PublicSupport::CCircBufDataBase<CSoePoint> soeDB_;
	PublicSupport::CCircBufDataBase<CYcVarPoint> ycvarDB_;

	//comm points
	std::vector<share_terminal_ptr> terminals_;
	std::vector<share_pristation_ptr> pristations_;
	share_bf533_ptr bf533_;

	//sig
	CmdRecallSignalType SubAliveSig_;
	CmdRecallSignalType SubTemporarySig_;
	CmdRecallSignalType EventStatusSig_;         //事件状态输出信号
	SigConnection EventStatusConnection_;        

	//事件状态注册的标识点号
	int YxReisterNO_;
	int YkRegisterNO_;
	int SelfRegisterNO_;

	//record data
	boost::scoped_ptr<CYkRecord> YkRecordPtr_;
	boost::scoped_ptr<CSoeRecord> SoeRecordPtr_;
	boost::scoped_ptr<CCosRecord> CosRecordPtr_;

	bool bSynTimeImmediately_;
	bool bCallDataImmediately_;
	bool bAddGeneralCmd_;
	bool bAddSpeCmd_;
};

};//namespace DataBase

