#pragma once
#include <boost/enable_shared_from_this.hpp>
#include "CommPoint.h"
#include "YcDB.h"
#include "YxDB.h"
#include "YmDB.h"
#include "YkDB.h"
#include "LineDB.h"

namespace PublicSupport
{
	class CSpaceRouteTab;
}

namespace DataBase {

const int CauseActiveData = 5;

const short NormalTerminal = 1;
const short BF533Terminal = 2;

class CSubStation;

class CYxPoint;
class CYcPoint;
class CYkPoint;
class CYmPoint;
class CCosPoint;
class CSoePoint;
class CFaultPoint;
class CLine;

class CYkRecord;
class CSoeRecord;
class CCosRecord;
class CFaultRecord;
class CYcHisRecord;

class CYcStatRecord;

using namespace boost::posix_time;

struct stSE_TIMEPOINT
{
	ptime startTime;
	ptime endTime;
};

class CTerminal :
	public CCommPoint,
	public boost::enable_shared_from_this<CTerminal>
{
public:
	//CTerminal();
	CTerminal(boost::asio::io_service & io_service,CSubStation & sub);
	virtual ~CTerminal(void);

	int InitTerminal(FileSystem::CMarkup & xml);
	void UnInitTerminal();

	virtual int InitLocalServices(CmdRecallSlotType slotVal,bool SlotReady);
	virtual void UnInitLocalServices();
	
	//yx api
	size_t getYxSum();
	size_t getSrcYxSum();
	size_t getRecvYxSum();
	int setYxSum(size_t val);
	typeYxval getOriYxVal(size_t index);
	int setOriYxVal(size_t index, typeYxval val,bool bCheckYxVar = false);
	int SaveOriYxVal(size_t index,typeYxval val,bool bCheckYxVar = false);
	typeYxtype getYxType(size_t index);
	int setYxType(size_t index,typeYxtype val);
	int SaveYxType(size_t index,typeYxtype val);
	bool getYxPolar(size_t index);
	int setYxPolar(size_t index,bool val);
	typeYxQuality getYxQuality(size_t index);
	int setYxQuality(size_t index,typeYxQuality val);
	int SaveYxQuality(size_t index,typeYxQuality val);
	typeFinalYcval getFinalYxVal(size_t index);
	bool getActiveCommYx();
	void setActiveCommYx(bool val);
	int AddEffectYxIndex(size_t index,size_t val);
	int getEffectYxSum(size_t index);
	int getEffectYxIndex(size_t index,int val);

	//yc api
	size_t getYcSum();
	//size_t getSrcYcSum();
	size_t getRecvYcSum();
	int setYcSum(size_t val);
	typeYcval getOriYcVal(size_t index);
	int setOriYcVal(size_t index,typeYcval val,bool bCheckYcVar = false);
	int SaveOriYcVal(size_t index,typeYcval val,bool bCheckYcVar = false);
	typeYcplus getYcPlus(size_t index);
	int setYcPlus(size_t index,typeYcplus val);
	typeYcmul getYcMul(size_t index);
	int setYcMul(size_t index,typeYcmul val);
	typeYcquality getYcQuality(size_t index);
	int setYcQuality(size_t index,typeYcquality val);
	int SaveYcQuality(size_t index,typeYcquality val);
	typeYcdead getYcDeadLimit(size_t index);
	int setYcDeadLimit(size_t index,typeYcdead val);
	typeFinalYcval getFinalYcVal(size_t index,bool bUseMul = false);
	CYcPoint * loadYcPointPtr(int index);
	CYcPoint * getYcPointPtr(int index);

	//yk api
	size_t getYkSum();
	//size_t getSrcYkSum();
	size_t getRecvYkSum();
	int setYkSum(size_t val);
	//typeYkstatus getYkStatus(size_t index);
	//int setYkStatus(size_t index,typeYkstatus val);
	//int saveYkStatus(size_t index,typeYkstatus val);
	typeYktype getYkType(size_t index);
	int setYkType(size_t index,typeYktype val);
	int saveYkType(size_t index,typeYktype val);
	bool getbHYkDouble(size_t index);
	int setbHYkDouble(size_t index,bool val);
	bool getbSYkDouble(size_t index);
	int setbSYkDouble(size_t index,bool val);
	int AddYkSelCmd(int index,bool bCloseOrOpen);
	int AddYkExeCmd(int index,bool bCloseOrOpen);
	int AddYkCancelCmd(int index,bool bCloseOrOpen = true);
	CYkPoint * loadYkPointPtr(int index);
	CYkPoint * getYkPointPtr(int index);

	//ym api
	size_t getYmSum();
	//size_t getSrcYmSum();
	size_t getRecvYmSum();
	int setYmSum(size_t val);
	typeYmval getOriYmVal(size_t index);
	int setOriYmVal(size_t index,typeYmval val);
	int SaveOriYmVal(size_t index,typeYmval val);
	typeYmquality getYmQuality(size_t index);
	int setYmQuality(size_t index,typeYmquality val);
	int SaveYmQuality(size_t index,typeYmquality val);

	//line api
	size_t getLineSum();
	CLine * getLinePtr(int index);

	//cos api
	int putCosPoint(size_t yxIndex,typeYxval val,typeYxtype yxType,typeYxQuality yxQuality = 0);
	int SaveCosPoint(size_t yxIndex,typeYxval val,typeYxtype yxType,typeYxQuality yxQuality = 0);

	//soe api
	int putSoePoint(size_t yxIndex,typeYxval val,typeYxtype yxType,ptime time,typeYxQuality yxQuality = 0);
	int SaveSoePoint(size_t yxIndex,typeYxval val,typeYxtype yxType,ptime time,typeYxQuality yxQuality = 0);

	//ycvar api
	int putYcvarPoint(size_t ycIndex,typeYcval val,typeYcquality ycQuality = 0,ptime time = ptime());
	int SaveYcvarPoint(size_t ycIndex,typeYcval val,typeYcquality ycQuality = 0,ptime time = ptime());

	//fault api
	int putFaultPoint(size_t faultIndex,typeYcval val,ptime time);
	int SaveFaultPoint(size_t faultIndex,typeYcval val,ptime time);

	//虚函数接口
	//virtual void AddSelfPointToProtocol();
	//virtual void AddSelfPointToTcpServerPtr();
	//virtual int ResetTimerRecv(size_t LostRecvTimeOut);
	//virtual void handle_timerRecv(const boost::system::error_code& error);
	//virtual int ConnectCmdRecallSig();
	virtual SigConnection ConnectSubAliveSig(CmdRecallSlotType slotVal);
	virtual SigConnection ConnectSubTempSig(CmdRecallSlotType slotVal);
	virtual share_commpoint_ptr getSelfPtr();
	virtual void ClearDataBaseQuality(bool active);

	//xml cfg api
	virtual void SaveXmlCfg(FileSystem::CMarkup & xml);
	virtual int LoadXmlCfg(FileSystem::CMarkup & xml);

	bool getActiveCommQuality();
	int setActiveCommQuality(bool val);

	bool getActiveInitQuality();
	int setActiveInitQuality(bool val);
	
	//protocol api
	bool getCurFcbFlag();

	//tab api
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> getYxTabPtr();
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> getYcTabPtr();
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> getYkTabPtr();
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> getYmTabPtr();

	//terminal type
	unsigned char getTerminalType();

	//comm active api
	virtual bool getCommActive();
	virtual void setCommActive(bool val);
	bool getCommActieBackup();
	void setCommActiveBackup(bool val);

	//frame api
	int CallData();
	int SynTime();

	int AddGeneralCmd(Protocol::CCmd cmdVal);
private:
	//index transform
	int DstyxToSrcyx(size_t val);
	int SrcyxToDstyx(size_t val);
	int DstycToSrcyc(size_t val);
	int SrcycToDstyc(size_t val);
	int DstykToSrcyk(size_t val);
	int SrcykToDstyk(size_t val);
	int DstymToSrcym(size_t val);
	int SrcymToDstym(size_t val);

	std::string TransActiveCommYxToString(bool val);
	bool TransActiveCommFromString(std::string val);

	//record data api
	int AddYkRecord(unsigned char recordType,int yk_no,bool bCloseOrOpen);
	int AddCosRecord(const CCosPoint & cos);
	int AddSoeRecord(const CSoePoint & soe);
	int AddFaultRecord(const CFaultPoint & fault);
	int AddHisYcRecord();
	int AddStatYcRecord();

	//timer api
	void handle_timerYcHisCycle(const boost::system::error_code& error);
	void ResetTimerYcHisCycle(bool bContinue);
	void handle_timerYcStatCycle(const boost::system::error_code& error);
	void ResetTimerYcStatCycle(bool bContinue);

	//event api
	void TrigCosEvent(size_t index,typeYxval val,bool bSingleType = true);
	void TrigSoeEvent(size_t index,typeYxval val,boost::posix_time::ptime time,bool bSingleType = true);
	public:
	void TrigYcSendByTimeEvent(int num);

protected:
	//comm active backup
	bool bCommActiveBackup_;

	//terminal type
	unsigned char terminalType_;

private:
	enum
	{
		DefaultYcCycleTime = 15, //遥测历史记录时间，单位：分钟
	};

	bool bSynTimeImmediately_;
	bool bCallDataImmediately_;
	bool bAddGeneralCmd_;
	bool bAddSpeCmd_;

	bool bActiveCommYX_;                     //激活终端通讯状态遥信
	bool bActiveCommQuality_;                //使终端的数据品质描述受通讯状态的影响
	bool bActiveInitQuality_;                //使终端的数据品质描述受初始化标志:CCommPoint::bInitCommPoint_影响

	CSubStation & sub_;

	//PublicSupport::CDynamicArrayDataBase<CYxPoint> yxDB_;
	//PublicSupport::CDynamicArrayDataBase<CYcPoint> ycDB_;
	//PublicSupport::CDynamicArrayDataBase<CYkPoint> ykDB_;
	//PublicSupport::CDynamicArrayDataBase<CYmPoint> ymDB_;
	//PublicSupport::CDynamicArrayDataBase<CLine> lineDB_;

	CYxDB yxDB_;
	CYcDB ycDB_;
	CYkDB ykDB_;
	CYmDB ymDB_;
	CLineDB lineDB_;

	boost::shared_ptr<PublicSupport::CSpaceRouteTab> yxTab_;
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> ycTab_;
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> ykTab_;
	boost::shared_ptr<PublicSupport::CSpaceRouteTab> ymTab_;
	int UseYxTabIndex_;
	int UseYcTabIndex_;
	int UseYmTabIndex_;
	int UseYkTabIndex_;

	//record data
	boost::scoped_ptr<CYkRecord> YkRecordPtr_;
	boost::scoped_ptr<CSoeRecord> SoeRecordPtr_;
	boost::scoped_ptr<CCosRecord> CosRecordPtr_;
	boost::scoped_ptr<CFaultRecord> FaultRecordPtr_;
	boost::scoped_ptr<CYcHisRecord> YcHisRecordPtr_;
	boost::scoped_ptr<CYcStatRecord> YcStatRecordPtr_;

	//timer
	int YcHisCycleTime_;
	boost::scoped_ptr<boost::asio::deadline_timer> YcHisTimerPtr_;
	int YcStatCycleTime_;
	boost::scoped_ptr<boost::asio::deadline_timer> YcStatTimerPtr_;
};

}; //namespace DataBase

